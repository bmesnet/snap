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


/*
 * Decouple the entries to maintain the multihash table from the data
 * in table_del, since we do not want to transfer empty entries over the
 * PCIe bus to the card.
 */

//Some local static variables are used.
static int card_no = 0;
static unsigned int timeout = 10;
snap_action_flag_t action_irq = 0;
static struct snap_card *card = NULL;
static struct snap_action *action = NULL;
static char device[128];
static struct snap_job cjob;
static struct acid_job jin;
static struct acid_job jout;
int verbose_flag = 0;

void closings()
{
	snap_detach_action(action);
	snap_card_free(card);
}

unsigned int openings(int card_in, int timeout_in, snap_action_flag_t action_irq)
{
	timeout = timeout_in;
	card_no = card_in;
	/*
	 * Apply for exclusive action access for action type 0xC0FE.
	 * Once granted, MMIO to that action will work.
	 */

	snprintf(device, sizeof(device)-1, "/dev/cxl/afu%d.0s", card_no);
	card = snap_card_alloc_dev(device, SNAP_VENDOR_ID_IBM,
				   SNAP_DEVICE_ID_SNAP);
	if (card == NULL) {
		fprintf(stderr, "err: failed to open card %u: %s\n",
			card_no, strerror(errno));
		fprintf(stderr, ">> Run with no FPGA? add SNAP_CONFIG=CPU before the command\n");
		exit(EXIT_FAILURE);
	}

	action = snap_attach_action(card, ACID_ACTION_TYPE, action_irq, 60);
	if (action == NULL) {
		fprintf(stderr, "err: failed to attach action %u: %s\n",
			card_no, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return 0;
}

static void snap_prepare_acid(struct snap_job *cjob,
				  struct acid_job *jin,
				  struct acid_job *jout,
				  const param_table_t *p_del, ssize_t p_del_size,
				  const table_del_t *t_del, ssize_t t_del_size,
				  const param_table_t *p_req, size_t p_req_size,
				  const table_req_t *t_req, size_t t_req_size,
				  bitset_t *bs, size_t bs_size, 
				  hashtable_t *h, size_t h_size)
{
	snap_addr_set(&jin->p_del, p_del, p_del_size,
		      SNAP_ADDRTYPE_HOST_DRAM,
		      SNAP_ADDRFLAG_ADDR | SNAP_ADDRFLAG_SRC);
	snap_addr_set(&jin->t_del, t_del, t_del_size,
		      SNAP_ADDRTYPE_HOST_DRAM,
		      SNAP_ADDRFLAG_ADDR | SNAP_ADDRFLAG_SRC);

	snap_addr_set(&jin->p_req, p_req, p_req_size,
		      SNAP_ADDRTYPE_HOST_DRAM,
		      SNAP_ADDRFLAG_ADDR | SNAP_ADDRFLAG_SRC);
	snap_addr_set(&jin->t_req, t_req, t_req_size,
		      SNAP_ADDRTYPE_HOST_DRAM,
		      SNAP_ADDRFLAG_ADDR | SNAP_ADDRFLAG_SRC);

	snap_addr_set(&jin->bs, bs, bs_size,
		      SNAP_ADDRTYPE_HOST_DRAM,
		      SNAP_ADDRFLAG_ADDR | SNAP_ADDRFLAG_DST);

	/* FIXME Assumptions where there is free DRAM on the card ... */
	snap_addr_set(&jin->hashtable, h, h_size,
		      SNAP_ADDRTYPE_CARD_DRAM,
		      SNAP_ADDRFLAG_ADDR | SNAP_ADDRFLAG_DST |
		      SNAP_ADDRFLAG_END);

        //jin->t_del_processed = 0;
        //jin->t_req_processed = 0; 

	snap_job_set(cjob, jin, sizeof(*jin), jout, sizeof(*jout));
}

int snap_acid( param_table_t *p_del, ssize_t p_del_size, table_del_t *t_del, ssize_t t_del_size,
	  param_table_t *p_req, size_t p_req_size, table_req_t *t_req, size_t t_req_size,
	  bitset_t *bs, size_t bs_size, hashtable_t *h, size_t h_size)
{
int rc;

		snap_prepare_acid(&cjob, &jin, &jout,
				      p_del, p_del_size,
				      t_del, t_del_size,
				      p_req, p_req_size,
				      t_req, t_req_size,
				      bs, bs_size,
				      h, h_size);
		// debug information
		//pr_info("Job Input:\n");
		//__hexdump(stderr, &jin, sizeof(jin));
		//table_del_dump(t_del, t_del_size);
		
       		//printf("[snap_acid]>> p_del_size=%ld t_del_size=%ld p_req_size=%ld t_req_size=%ld\n",
                //	p_del_size, t_del_size, p_req_size, t_req_size);
 
       		//jin.t_del_processed = 1; 	/* set the action to use the del table */
		rc = snap_action_sync_execute_job(action, &cjob, timeout);

                //if(t_req_size != 0) {
                //    printf("\nBitset dump: Byte0 - 1 - 2 -... \n");
                //    __hexdump(stdout, bs, sizeof(bs));
                //}

return rc;
}

