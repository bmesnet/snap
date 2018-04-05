/*
 * Copyright 2017, International Business Machines
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
 * Example for HashJoin Algorithm
 *
 * Copyright (C) 2017 Rosetta.org
 *
 * Permission is granted to copy, distribute and/or modify this document
 * under the terms of the GNU Free Documentation License, Version 1.3
 * or any later version published by the Free Software Foundation;
 * with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.
 * A copy of the license is included in the section entitled "GNU
 * Free Documentation License".
 */

/*
 * HLS Adoptations and other additions
 *
 * Copyright 2017, International Business Machines
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <libsnap.h>
#include <snap_internal.h>
#include <snap_acid.h>
#include <snap_tools.h>
//#define VERBOSE_MODE
#define VERBOSE_MODE_MIN

static int mmio_read32(struct snap_card *card,
		       uint64_t offs, uint32_t *data)
{
	act_trace("  %s(%p, %llx, %x)\n", __func__, card,
		  (long long)offs, *data);
	return 0;
}

/*
 * The strcmp() function compares the two ROW_ID s1 and s2. It
 * returns 0 if s1 equal to s2, or the number of differences if any
 */
static int hashkey_cmp(const ROW_ID_t s1, const ROW_ID_t s2)
{
	int rc = 0;

	(s1.otid == s2.otid ? rc = rc : rc++); 
	(s1.bucketProperty == s2.bucketProperty ? rc = rc : rc++); 
	(s1.rowId == s2.rowId ? rc = rc : rc++); 

	return rc;
}

static void hashkey_cpy(ROW_ID_t *dst, ROW_ID_t src)
{
	dst->otid           = src.otid;
	dst->bucketProperty = src.bucketProperty;
	dst->rowId          = src.rowId;
}

/* Create a new hashtable. */
static void ht_init(hashtable_t *ht)
{
	unsigned int i;

	for (i = 0; i < HT_SIZE; i++) {
		entry_t *entry = &ht->table[i];

		entry->used = 0;
	}
}

/* Hash a key for a particular hash table. */
static int ht_hash(ROW_ID_t key)
{
	// 4GB = 2^32
	return key.rowId % HT_SIZE;
}

/**
 * Insert a key into a hash table.
 */
static int ht_set(hashtable_t *ht, ROW_ID_t key)
{
	int rc;
	unsigned int i, j;
	unsigned int bin = 0;

	bin = ht_hash(key);
#ifdef VERBOSE_MODE
        printf("\n B: { key.ROW_ID=(%ld, %d, %ld) }\t",
               key.otid, key.bucketProperty, key.rowId);
#endif
	/* search if entry exists already */
	for (i = 0; i < HT_SIZE; i++) { 
		table_req_t *multi;
		entry_t *entry = &ht->table[bin];

		if (entry->used == 0) {	/* hey unused, we can have it */
#ifdef VERBOSE_MODE
			printf("key is %d, used %d <= 1rst entry", bin, entry->used);
#endif
			multi = &entry->multi[entry->used];
			hashkey_cpy(&multi->ROW_ID, key);
			entry->used++;
			return 0;
		}
		else {
	  	   for (j = 0; j < HT_MULTI; j++) {  /* test all multi entries */
			rc = hashkey_cmp(key, (entry->multi[j]).ROW_ID);

			if (rc == 0)  {		/* key already in ht */
#ifdef VERBOSE_MODE
				printf("key is %d, used %d <= entry already exist", bin, j);
#endif
				return 0;
			}
			else {     /* different value */ 
			   if(j == entry->used) {  /* insert new multi */
#ifdef VERBOSE_MODE
				printf("key is %d, used %d <= adding multi entry", bin, j);
#endif
				multi = &entry->multi[entry->used];
				hashkey_cpy(&multi->ROW_ID, key);
				entry->used++;
				return 0;
		   	   }
		   	}
		    }
		    if (entry->used == HT_MULTI) {
			/* double hash because of collision */
			/* try next one */
			/* FIXME: may be good to add a flag saying that next bin used  */
			/* FIXME: or continue incrementing used to know how many values */
#ifdef VERBOSE_MODE
			printf("row %d, used %d <= trying next row", bin, entry->used);
#endif 
			bin = (bin + 1) % HT_SIZE;
		    }
		} // end of else
	}

	printf("\n\n <<< ERROR >>> LAST LINE REACHED - OVERFLOW\n\n");
	return 0;
}

/**
 * Retrieve an array of values matching the key from a hash table.
 * Return the index and not the pointer to entry_t, since HLS does
 * not like that.
 *
 * Non-optimal double hash implementation: pick the next free entry.
 */
static int ht_get(hashtable_t *ht, ROW_ID_t key)
{
	int rc;
	unsigned int i, j;
	unsigned int bin = 0;
	entry_t *entry = NULL;

	bin = ht_hash(key);
#ifdef VERBOSE_MODE
        printf("\n R: { key.ROW_ID=(%ld, %d, %ld) }  ",
               key.otid, key.bucketProperty, key.rowId);
#endif


	/* search if entry exists already */
	for (i = 0; i < HT_SIZE; i++) {
		table_req_t *multi;

		entry = &ht->table[bin];

		if (entry->used == 0) { 	/* key not there */
#ifdef VERBOSE_MODE
			printf("key is %d, used %d <= no entries", bin, entry->used);
#endif
			return -1;
		}

	  	for (j = 0; j < entry->used; j++) {  /* test all multi entries */
			multi = &entry->multi[j];
			rc = hashkey_cmp(key, multi->ROW_ID);

			if (rc == 0) {		/* good key was found */
#ifdef VERBOSE_MODE
				printf("key is %d, position in multi %d(/%d) => FOUND", 
					bin, j, entry->used - 1);
#endif
				return bin;
			}
		}
		/* double hash */
		if (entry->used == HT_MULTI) {	/* try next one */
#ifdef VERBOSE_MODE
			printf("key is %d, used %d <= trying next row", bin, entry->used);
#endif
			bin = (bin + 1) % HT_SIZE;
		}
		else {
#ifdef VERBOSE_MODE
			printf("key is %d, scan to multi %d(/%d) <= not found", 
				bin, j - 1, entry->used - 1);
#endif
			return -1;
		}
	}

	return -1;
}
/*
 * #!/usr/bin/python
 * from collections import defaultdict
 *
 * def hash_write(table_del, index1):
 *     h = defaultdict(list)
 *     # hash phase
 *     for s in table_del:
 *        h[s[index1]].append(s)
 */

  static int hash_write(param_table_t *p_del, table_del_t *table_del, unsigned table_del_size,
                     unsigned int del_entries,
		     hashtable_t *h 
                     )
{
	unsigned int i, j;
	table_del_t *t_del;
	bool skip_ht_set;
       	//printf("\n (key used is ROW_ID.row_id modulo %d (HT_SIZE))", HT_SIZE);

	// hash phase 
	if(del_entries == UINT_MAX) { /*FIXME : find a smart way to reset the ht */
#ifdef VERBOSE_MODE_MIN
		printf("Initialization of the hash table requested\n");
#endif
		ht_init(h);
	}
	for (i = 0; i < table_del_size; i++) {
		skip_ht_set = false;
		t_del = &table_del[i];

		// Filtering is done on the txnid - then storing ROW_ID in hash table
        	//printf("\ndel_txnid=%3ld  ", t_del->del_txnid);
		if (t_del->del_txnid >= p_del->low_mark) {   //All txns < low-mark is included 
#ifdef VERBOSE_MODE_MIN
        		printf("\ndel_txnid=%3ld  ", t_del->del_txnid);
			printf("filtered:  del_txnid >= low-mark:%ld ",p_del->low_mark); 
#endif
			skip_ht_set = true; }
		if (t_del->del_txnid <  p_del->low_input) {  //All txns < low-input is excluded
#ifdef VERBOSE_MODE_MIN
        		printf("\ndel_txnid=%3ld  ", t_del->del_txnid);
			printf("filtered:  del_txnid < low-input:%ld ",  p_del->low_input); 
#endif
			skip_ht_set = true; }
		if (t_del->del_txnid >  p_del->high_mark) {  //All txns > high-mark are excluded 
#ifdef VERBOSE_MODE_MIN
        		printf("\ndel_txnid=%3ld  ", t_del->del_txnid);
			printf("filtered:  del_txnid > high-mark:%ld ",  p_del->high_mark); 
#endif
			skip_ht_set = true; }
		if (t_del->del_txnid >  p_del->high_input) { //All txns > high-input is excluded 
#ifdef VERBOSE_MODE_MIN
        		printf("\ndel_txnid=%3ld  ", t_del->del_txnid);
			printf("filtered:  del_txnid > high_input:%ld ", p_del->high_input); 
#endif
			skip_ht_set = true; }
		for (j = 0; j <  p_del->excluded_txn_num; j++) 
			if ( t_del->del_txnid == p_del->excluded_txn[j]) { //All txns in exclusions are excluded
#ifdef VERBOSE_MODE_MIN
        			printf("\ndel_txnid=%3ld  ", t_del->del_txnid);
				printf("filtered:  del_txnid is listed in excluded list"); 
#endif
				skip_ht_set = true;
			}

		if(!skip_ht_set)
			ht_set(h, t_del->ROW_ID);
#ifdef VERBOSE_MODE_MIN
		else
			printf(" => entry not added to hash table"); 
#endif
	}
	del_entries--;

	//ht_dump(h); // For debug purpose : dumps the hash table
	return 0;
}

  static int hash_read(param_table_t *p_req, table_req_t *table_req, unsigned table_req_size,
                     unsigned int req_entries,
                     bitset_t *bitset,
                     unsigned int bitset_number,
		     hashtable_t *h 
                     )
{
	unsigned int i, j;
	int bin;
	bool skip_ht_get;
	for(i = 0; i < 128; i++)
		bitset->bs[i] = 0xFF;
       	//printf("\n (key used is ROW_ID.row_id modulo %d (HT_SIZE))\n", HT_SIZE);

	for (i = 0; i < table_req_size; i++) {
		skip_ht_get = false;
		table_req_t *t_req = &table_req[i];
        	//printf("\n#%2d - ROW_ID=(%ld, %d, %ld) ", i, t_req->ROW_ID.otid, t_req->ROW_ID.bucketProperty, t_req->ROW_ID.rowId);

		if (t_req->ROW_ID.otid >= p_req->low_mark) { //All txns < low-mark is included 
#ifdef VERBOSE_MODE_MIN
        		printf("\n#%2d - ROW_ID=(%ld, %d, %ld) ", i, t_req->ROW_ID.otid, t_req->ROW_ID.bucketProperty, t_req->ROW_ID.rowId);
        		printf("- txn_id=%ld  ", t_req->ROW_ID.otid);
			printf("Req filtered:  otid >= low-mark:%ld",p_req->low_mark); 
#endif
			skip_ht_get = true; 
			skip_ht_get = true; }
		if (t_req->ROW_ID.otid >  p_req->high_mark) { //All txns > high-mark are excluded
#ifdef VERBOSE_MODE_MIN
        		printf("\n#%2d - ROW_ID=(%ld, %d, %ld) ", i, t_req->ROW_ID.otid, t_req->ROW_ID.bucketProperty, t_req->ROW_ID.rowId);
        		printf("- txn_id=%ld  ", t_req->ROW_ID.otid);
			printf("Req filtered:  otid > high-mark:%ld",  p_req->high_mark); 
#endif
			skip_ht_get = true; }
		for (j = 0; j <  p_req->excluded_txn_num; j++) 
			if ( t_req->ROW_ID.otid == p_req->excluded_txn[j]) { //All txns in exclusions are excluded
#ifdef VERBOSE_MODE_MIN
        			printf("\n#%2d - ROW_ID=(%ld, %d, %ld) ", i, t_req->ROW_ID.otid, t_req->ROW_ID.bucketProperty, t_req->ROW_ID.rowId);
        			printf("- txn_id=%ld  ", t_req->ROW_ID.otid);
				printf("Req filtered:  otid is listed in excluded list"); 
#endif
				skip_ht_get = true; 
			}

		if (!skip_ht_get) 
			bin = ht_get(h, t_req->ROW_ID);
		else
			bin = -1;
		if (bin == -1)
			continue;	// nothing found
		else {
			bitset_number++;
#ifdef VERBOSE_MODE_MIN
			printf("\nROW_ID %ld FOUND in table => CLEARING BIT %d (#%d)",  t_req->ROW_ID.rowId, i, (int)bitset_number);
#endif
			bitset->bs[i/8] &= ~(1 << i%8);
		}
	}
	req_entries--;

	//printf("\n => Number of BITS CLEARED = %d\n", (int)bitset_number);
	//__hexdump(stdout, &bitset->bs, sizeof(bitset->bs));
	return 0;
}
/*
static void print_job(struct acid_job *j)
{
	printf("Acid Job\n");
	printf("  t_del: %016llx %d bytes %ld entries\n",
	       (long long)j->t_del.addr, j->t_del.size,
	       j->t_del.size/sizeof(table_del_t));
	printf("  t_req: %016llx %d bytes %ld entries\n",
	       (long long)j->t_req.addr, j->t_req.size,
	       j->t_req.size/sizeof(table_req_t));

	printf("  h:  %016llx %d bytes %ld entries\n",
	       (long long)j->hashtable.addr, j->hashtable.size,
	       j->hashtable.size/sizeof(entry_t));

}
*/
static int action_main(struct snap_sim_action *action,
		       void *job, unsigned int job_len __unused)
{
	int rc;
	struct acid_job *hj = (struct acid_job *)job;
	param_table_t *p_del;
	table_del_t *t_del;
	param_table_t *p_req;
	table_req_t *t_req;
	bitset_t *bs;
	unsigned int bitset_number = 0;
	hashtable_t *h;


	//print_job(hj);

	p_del = (param_table_t *)hj->p_del.addr;
	if (!p_del) {
		printf("  p_del error \n");
		goto err_out;
	}
	t_del = (table_del_t *)hj->t_del.addr;
	if (!t_del || hj->t_del.size/sizeof(table_del_t) > TABLE_DEL_SIZE) {
		printf("ERROR: t_del.size/sizeof(table_del_t) = %ld entries > TABLE_DEL_SIZE = %d\n",
		       hj->t_del.size/sizeof(table_del_t), TABLE_DEL_SIZE);
		goto err_out;
	}

	p_req = (param_table_t *)hj->p_req.addr;
	if (!p_req) {
		printf("  p_req error \n");
		goto err_out;
	}
	t_req = (table_req_t *)hj->t_req.addr;
	if (!t_req || hj->t_req.size/sizeof(table_req_t) > TABLE_REQ_SIZE) {
		printf("ERROR: t_req.size/sizeof(table_req_t) = %ld entries > TABLE_REQ_SIZE = %d\n",
		       hj->t_req.size/sizeof(table_req_t), TABLE_REQ_SIZE);
		goto err_out;
	}

	bs = (bitset_t *)hj->bs.addr;

	h = (hashtable_t *)hj->hashtable.addr;
	if (hj->hashtable.size/sizeof(entry_t) > HT_SIZE) {
		printf("  hashtable.size/sizeof(entry_t) = %ld entries\n",
		       hj->hashtable.size/sizeof(entry_t));
		goto err_out;
	}

	// Build the hash table with deleted entries
	if(p_del->entries != 0) {
		//printf("\n<DBG>  Calling hash_write - p_del_entries= %ld\n", p_del->entries);
		rc = hash_write(p_del, t_del, hj->t_del.size/sizeof(table_del_t), 
                       p_del->entries, h);
	}

	// Read the hash table with 1024 entries blocks
	if(p_req->entries != 0) {
		//printf("\n<DBG>  Calling hash_read - p_req_entries=%ld\n", p_req->entries);
		rc = hash_read(p_req, t_req, hj->t_req.size/sizeof(table_req_t), 
                       p_req->entries,
                       bs, bitset_number, h);
		//printf("\n<DBG>  Bit set number =%d\n", (int)bitset_number);
		//__hexdump(stdout, bs, sizeof(bs));
	}

	if (rc == 0) {
		action->job.retc = SNAP_RETC_SUCCESS;
	} else
		action->job.retc = SNAP_RETC_FAILURE;
	return 0;

 err_out:
	action->job.retc = SNAP_RETC_FAILURE;
	return -1;
}

static struct snap_sim_action action = {
	.vendor_id = SNAP_VENDOR_ID_ANY,
	.device_id = SNAP_DEVICE_ID_ANY,
	.action_type = ACID_ACTION_TYPE,

	.job = { .retc = SNAP_RETC_FAILURE, },
	.state = ACTION_IDLE,
	.main = action_main,
	.priv_data = NULL,	/* this is passed back as void *card */
	.mmio_read32 = mmio_read32,

	.next = NULL,
};

static void _init(void) __attribute__((constructor));

static void _init(void)
{
	snap_action_register(&action);
}
