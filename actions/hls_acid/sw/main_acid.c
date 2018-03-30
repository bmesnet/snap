/*
 * Copyright 2017 International Business Machines
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 * Example to use the FPGA to do a hash join operation on two input
 * tables table_del_t and table_req_t resuling in a new combined table3_t.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <malloc.h>
#include <endian.h>
#include <asm/byteorder.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <libsnap.h>
#include <snap_tools.h>
#include <snap_s_regs.h>
#include <snap_acid.h>

static const char *version = GIT_VERSION;

/*
 * Decouple the entries to maintain the multihash table from the data
 * in table_del, since we do not want to transfer empty entries over the
 * PCIe bus to the card.
 */
static table_del_t t_del[TABLE_DEL_SIZE] __attribute__((aligned(ACID_ALIGN)));
static table_req_t t_req[TABLE_REQ_SIZE] __attribute__((aligned(ACID_ALIGN)));
static hashtable_t hashtable __attribute__((aligned(64)));
static bitset_t bs __attribute__((aligned(128)));

// Generating the txnid for the deleted txnid
static void table_del_fill(table_del_t *t_del, unsigned int t_del_entries)
{
	unsigned int i;

	printf(">> All values for txnid are generated modulo %d (MAX_TXNID_NUM)\n",  MAX_TXNID_NUM);
	for (i = 0; i < t_del_entries; i++) {
		t_del[i].del_txnid = (rand() % MAX_TXNID_NUM); 
		t_del[i].ROW_ID.otid = rand() % MAX_TXNID_NUM;
		t_del[i].ROW_ID.bucketProperty = 0;
		t_del[i].ROW_ID.rowId = rand() % MAX_TXNID_NUM;
	}
}
// Generating a list of exceptions (excluded txnids)
static void table_exception_fill(param_table_t *p_table)
{
	unsigned int i;

	p_table->excluded_txn_num = MAX_EXCL_TXN;
	printf(">> EXCLUDING %ld TXNID: ",  p_table->excluded_txn_num);

	for (i = 0; i < MAX_EXCL_TXN; i++) {
		p_table->excluded_txn[i] = (rand() % MAX_TXNID_NUM); 
		printf("%ld - ", p_table->excluded_txn[i]);
	}
	printf(" <<\n\n");
}
// Generating the txnid for the requests
static void table_req_fill(table_req_t *t_req, unsigned int t_req_entries)
{
	unsigned int i;

	printf(">> All values for txnid are generated modulo %d (MAX_TXNID_NUM)\n",  MAX_TXNID_NUM);
	for (i = 0; i < t_req_entries; i++) {
		t_req[i].ROW_ID.otid = rand() % MAX_TXNID_NUM;
		t_req[i].ROW_ID.bucketProperty = 0;
		t_req[i].ROW_ID.rowId = rand() % MAX_TXNID_NUM;
	}
}

static inline
ssize_t file_size(const char *fname)
{
	int rc;
	struct stat s;

	rc = lstat(fname, &s);
	if (rc != 0) {
		fprintf(stderr, "err: Cannot find %s!\n", fname);
		return rc;
	}
	return s.st_size;
}

static inline ssize_t
file_read(const char *fname, uint8_t *buff, size_t len)
{
	int rc;
	FILE *fp;

	if ((fname == NULL) || (buff == NULL) || (len == 0))
		return -EINVAL;

	fp = fopen(fname, "r");
	if (!fp) {
		fprintf(stderr, "err: Cannot open file %s: %s\n",
			fname, strerror(errno));
		return -ENODEV;
	}
	rc = fread(buff, len, 1, fp);
	if (rc == -1) {
		fprintf(stderr, "err: Cannot read from %s: %s\n",
			fname, strerror(errno));
		fclose(fp);
		return -EIO;
	}

	fclose(fp);
	return rc;
}

/**
 * @brief	prints valid command line options
 *
 * @param prog	current program's name
 */
static void usage(const char *prog)
{
	printf("Usage: %s [-h] [-V, --version]\n"
	       "  -C, --card <cardno> can be (0...3)\n"
	       "  -t, --timeout <timeout>  Timefor for job completion. (default 10 sec)\n"
	       "  -D, --t_del-entries <items> Entries in table_del.\n"
	       "  -R, --t_req-entries <items> Entries in table_req.\n"
	       "  -s, --seed <seed>        Random seed to enable recreation.\n"
	       "  -N, --no irq             Disable Interrupts\n"
	       "\n"
	       "Example:\n"
	       "  snap_hashjoin ...\n"
	       "\n",
	       prog);
}

/**
 * Read accelerator specific registers. Must be called as root!
 */
int main(int argc, char *argv[])
{
	int ch, rc = 0;
	int card_no = 0;
	//struct snap_card *card = NULL;
	//struct snap_action *action = NULL;
	unsigned int timeout = 10;
	struct timeval etimeD, stimeD;
	struct timeval etimeR, stimeR;
	int exit_code = EXIT_SUCCESS;
	param_table_t p_req_t;
	param_table_t p_del_t;
	unsigned int t_del_entries = 25;
	unsigned int t_del_tocopy = 0;
	unsigned int t_req_entries = 23;
	unsigned int t_req_tocopy = 0;
	unsigned int seed = 1974;
	snap_action_flag_t action_irq = (SNAP_ACTION_DONE_IRQ | SNAP_ATTACH_IRQ);

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{ "card",	 required_argument, NULL, 'C' },
			{ "timeout",	 required_argument, NULL, 't' },
			{ "t_del-entries",	 required_argument, NULL, 'D' },
			{ "t_req-entries",	 required_argument, NULL, 'R' },
			{ "seed",	 required_argument, NULL, 's' },
			{ "version",	 no_argument,	    NULL, 'V' },
			{ "help",	 no_argument,	    NULL, 'h' },
			{ "no_irq",	 no_argument,	    NULL, 'N' },
			{ 0,		 no_argument,	    NULL, 0   },
		};

		ch = getopt_long(argc, argv,
				 "s:D:R:C:t:VhI",
				 long_options, &option_index);
		if (ch == -1)	/* all params processed ? */
			break;

		switch (ch) {
		/* which card to use */
		case 'C':
			card_no = strtol(optarg, (char **)NULL, 0);
			break;
		case 't':
			timeout = strtol(optarg, (char **)NULL, 0);
			break;
		case 'D':
			t_del_entries = strtol(optarg, (char **)NULL, 0);
			break;
		case 'R':
			t_req_entries = strtol(optarg, (char **)NULL, 0);
			break;
		case 's':
			seed = strtol(optarg, (char **)NULL, 0);
			break;
		case 'V':
			printf("%s\n", version);
			exit(EXIT_SUCCESS);
		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
			break;
		case 'N':
			action_irq = 0;
			break;
		default:
			usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (optind != argc) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	srand(seed);

	////////////////////////////////////////////
	//Card No., timeout, interrupt has to be set.
	//If openings() fails, it exits before malloc any arrays. 
	openings(card_no, timeout, action_irq);
	

/* Using the -D value command line argument, we can fill the
 * hash table with "bursts" of TABLE_DEL_SIZE(=32) elements
 */
	printf("\n>> BUILDING THE HASH TABLE FOR %d ENTRIES (bursts of %d entries)\n", 
		t_del_entries, TABLE_DEL_SIZE);
	//fill he params structure
	p_req_t.entries = 0;
	t_req_tocopy = 0;

	p_del_t.entries = t_del_entries;
	p_del_t.low_mark = DEL_LOW_MARK;
	p_del_t.high_mark = DEL_HIGH_MARK;
	p_del_t.low_input = DEL_LOW_INPUT;
	p_del_t.high_input = DEL_HIGH_INPUT;
	printf(">> Parameters used are: LOW-MARK=%d HIGH-MARK=%d LOW-INPUT=%d HIGH-INPUT=%d\n", 
		DEL_LOW_MARK, DEL_HIGH_MARK, DEL_LOW_INPUT, DEL_HIGH_INPUT);
	        table_exception_fill(&p_del_t);

	printf("\n>> INITI of the HASH TABLE <<\n");
	p_del_t.entries = UINT_MAX;
	snap_acid( &p_del_t, sizeof(param_table_t),
			    t_del, t_del_tocopy * sizeof(table_del_t),
			    &p_req_t, sizeof(param_table_t),
			    t_req, t_req_tocopy * sizeof(table_req_t),
			    &bs, sizeof(bs),
			    &hashtable, sizeof(hashtable));

	p_del_t.entries = t_del_entries;

	gettimeofday(&stimeD, NULL);
	while (t_del_entries != 0) {

		t_del_tocopy = MIN(ARRAY_SIZE(t_del), t_del_entries);
		table_del_fill(t_del, t_del_tocopy);
	
		snap_acid( &p_del_t, sizeof(param_table_t),
			    t_del, t_del_tocopy * sizeof(table_del_t),
			    &p_req_t, sizeof(param_table_t),
			    t_req, t_req_tocopy * sizeof(table_req_t),
			    &bs, sizeof(bs),
			    &hashtable, sizeof(hashtable));
		t_del_entries -= t_del_tocopy;
	}
	gettimeofday(&etimeD, NULL);

	printf("\n\n>> LOOKING FOR %d ENTRIES (bursts of %d entries)\n\n", 
		t_req_entries, TABLE_REQ_SIZE);
	//fill he params structure
	p_del_t.entries = 0;
	t_del_tocopy = 0;

	p_req_t.entries = t_req_entries;
	p_req_t.low_mark = REQ_LOW_MARK;
	p_req_t.high_mark = REQ_HIGH_MARK;
	//p_req_t.low_input = 1;     // unused
	//p_req_t.high_input = 999;  // unsed
	printf(">> Parameters used are: LOW-MARK=%d HIGH-MARK=%d\n", 
		REQ_LOW_MARK, REQ_HIGH_MARK);
	table_exception_fill(&p_req_t);

	gettimeofday(&stimeR, NULL);
	while (t_req_entries != 0) {
		t_req_tocopy = MIN(ARRAY_SIZE(t_req), t_req_entries);

		table_req_fill(t_req, t_req_tocopy);

		snap_acid( &p_del_t, sizeof(param_table_t),
			   t_del, t_del_tocopy * sizeof(table_del_t),
			   &p_req_t, sizeof(param_table_t),
			   t_req, t_req_tocopy * sizeof(table_req_t),
			   &bs, sizeof(bs),
			   &hashtable, sizeof(hashtable));


		if (rc != 0) {
			fprintf(stderr, "err: job execution %d: %s!\n", rc,
				strerror(errno));
			goto out_error;
		}
		else {
			printf("\nBitset dump:\n      Byte |1-0|-|3-2|-|4-5|-. \n");
			__hexdump(stderr, &bs, sizeof(bs));
		}

/*
		if (cjob.retc != SNAP_RETC_SUCCESS)  {
			fprintf(stderr, "err: job retc %x!\n", cjob.retc);
			goto out_error;
		}
*/
		t_req_entries -= t_req_tocopy;
	}
	gettimeofday(&etimeR, NULL);

	printf("With default arguments, the expected result for Bitset is:\n"\
		" 00000000: f6 f7 f9 ff ff ff ff ff ff ff ff ff ff ff ff ff\n");

	fprintf(stderr, "ReturnCode: %s\n"
		"ACID Deletion lookup took %lld usec\n"
		"  building hash table took %lld usec\n" 
		"  looking for values  took %lld usec\n", 
	//	(cjob.retc == SNAP_RETC_SUCCESS ? "SUCCESS" : "FAILED"),
		(rc == 0 ? "SUCCESS" : "FAILED"),
		(long long)timediff_usec(&etimeR, &stimeD),
		(long long)timediff_usec(&etimeD, &stimeD),
		(long long)timediff_usec(&etimeR, &stimeR));

	closings();
	exit(exit_code);

 out_error:
	closings();
	exit(EXIT_FAILURE);
}
