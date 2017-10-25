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
        printf("key.ROW_ID=(%ld, %d, %ld) }\t",
               key.otid, key.bucketProperty, key.rowId);

	/* search if entry exists already */
	for (i = 0; i < HT_SIZE; i++) { 
		table_req_t *multi;
		entry_t *entry = &ht->table[bin];

		if (entry->used == 0) {	/* hey unused, we can have it */
			printf("row %d, used %d <= 1rst entry\n", bin, entry->used);
			multi = &entry->multi[entry->used];
			hashkey_cpy(&multi->ROW_ID, key);
			entry->used++;
			return 0;
		}
		else {
	  	   for (j = 0; j < HT_MULTI; j++) {  /* test all multi entries */
			rc = hashkey_cmp(key, (entry->multi[j]).ROW_ID);

			if (rc == 0)  {		/* key already in ht */
				printf("row %d, used %d <= entry already exist\n", bin, j);
				return 0;
			}
			else {     /* different value */ 
			   if(j == entry->used) {  /* insert new multi */
				printf("row %d, used %d <= adding multi entry\n", bin, j);
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
			printf("row %d, used %d <= trying next row\n", bin, entry->used);
			bin = (bin + 1) % HT_SIZE;
		    }
		} // end of else
	}

	printf("LAST LINE REACHED - OVERFLOW\n");
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
        printf("Looking for { key.ROW_ID=(%ld, %d, %ld) }\t",
               key.otid, key.bucketProperty, key.rowId);


	/* search if entry exists already */
	for (i = 0; i < HT_SIZE; i++) {
		table_req_t *multi;

		entry = &ht->table[bin];

		if (entry->used == 0) { 	/* key not there */
			printf("row %d, used %d <= no entries\n", bin, entry->used);
			return -1;
		}

	  	for (j = 0; j < entry->used; j++) {  /* test all multi entries */
			multi = &entry->multi[j];
			rc = hashkey_cmp(key, multi->ROW_ID);

			if (rc == 0) {		/* good key was found */
				printf("row %d, multi %d <= entry FOUND\n", bin, j);
				return bin;
			}
		}
		/* double hash */
		if (entry->used == HT_MULTI) {	/* try next one */
			printf("row %d, used %d <= trying next row\n", bin, entry->used);
			bin = (bin + 1) % HT_SIZE;
		}
		else {
			printf("row %d, multi %d <= not found\n", bin, j);
			return -1;
		}
	}

	return -1;
}
/*
 * #!/usr/bin/python
 * from collections import defaultdict
 *
 * def hashJoin(table_del, index1, table_req, index2):
 *     h = defaultdict(list)
 *     # hash phase
 *     for s in table_del:
 *        h[s[index1]].append(s)
 *     # join phase
 *     return [(s, r) for r in table_req for s in h[r[index2]]]
 *
 * for row in hashJoin(table_del, 1, table_req, 0):
 *     print(row)
 *
 * Output: Bitset of 1024 bits 
 */

  static int hash_fct(table_del_t *table_del, unsigned table_del_size,
                     unsigned int del_entries,
                     table_req_t *table_req, unsigned table_req_size,
                     unsigned int req_entries,
                     bitset_t *bitset,
                     unsigned int bitset_number,
		     hashtable_t *h 
                     )
{
	unsigned int i;
	table_del_t *t_del;


	bitset->bs_LSB = 0;
	bitset->bs_MSB = 0;

	if(del_entries != 0) {
	   // hash phase 
	   if(del_entries == UINT_MAX) { /*FIXME : find a smart way to reset the ht */
		printf("Initialization of the hash table requested\n");
		ht_init(h);
	   }
	   for (i = 0; i < table_del_size; i++) {
		t_del = &table_del[i];

		if (t_del->del_txnid == LAST_TXNID_ELMT) // testing whatever need to be tested
			continue;

		ht_set(h, t_del->ROW_ID);
	   }
	   del_entries--;
	}

	ht_dump(h); 

	if(req_entries != 0) {
	   for (i = 0; i < table_req_size; i++) {
		int bin;
		table_req_t *t_req = &table_req[i];

		bin = ht_get(h, t_req->ROW_ID);
		if (bin == -1)
			continue;	// nothing found
		else {
			bitset_number++;
			printf("=> setting bitset request %d  (#%d)\n", i, (int)bitset_number);
			if (i < 512)
				bitset->bs_LSB = bitset->bs_LSB | (1 << i);
			else
				bitset->bs_MSB = bitset->bs_MSB | (1 << i);
		}
	   }
	   req_entries--;
	}

	return 0;
}

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

static int action_main(struct snap_sim_action *action,
		       void *job, unsigned int job_len __unused)
{
	int rc;
	struct acid_job *hj = (struct acid_job *)job;
	table_del_t *t_del;
	table_req_t *t_req;
	bitset_t *bs;
	unsigned int bitset_number = 0;
	hashtable_t *h;

	print_job(hj);

	t_del = (table_del_t *)hj->t_del.addr;
	if (!t_del || hj->t_del.size/sizeof(table_del_t) > TABLE_DEL_SIZE) {
		printf("  t_del.size/sizeof(table_del_t) = %ld entries\n",
		       hj->t_del.size/sizeof(table_del_t));
		goto err_out;
	}

	t_req = (table_req_t *)hj->t_req.addr;
	if (!t_req || hj->t_req.size/sizeof(table_req_t) > TABLE_REQ_SIZE) {
		printf("  t_req.size/sizeof(table_req_t) = %ld entries\n",
		       hj->t_req.size/sizeof(table_req_t));
		goto err_out;
	}

	bs = (bitset_t *)hj->bs.addr;

	h = (hashtable_t *)hj->hashtable.addr;
	if (hj->hashtable.size/sizeof(entry_t) > HT_SIZE) {
		printf("  hashtable.size/sizeof(entry_t) = %ld entries\n",
		       hj->hashtable.size/sizeof(entry_t));
		goto err_out;
	}

	rc = hash_fct(t_del, hj->t_del.size/sizeof(table_del_t), 
                       hj->t_del_processed,
                       t_req, hj->t_req.size/sizeof(table_req_t), 
                       hj->t_req_processed,
                       bs, bitset_number, h);
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
