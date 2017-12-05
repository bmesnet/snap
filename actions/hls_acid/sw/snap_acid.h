#ifndef __SNAP_ACID_H__
#define __SNAP_ACID_H__

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

#include <stdint.h>
#include <stdio.h>
#include <libsnap.h>
#include <action_acid.h>

//struct snap_card *card = NULL;

void closings(void);
unsigned int openings(int card_in, int timeout_in, snap_action_flag_t action_irq);
int snap_acid( param_table_t *p_del, ssize_t p_del_size, table_del_t *t_del, ssize_t t_del_size,
          param_table_t *p_req, size_t p_req_size, table_req_t *t_req, size_t t_req_size,
          bitset_t *bs, size_t bs_size, hashtable_t *h, size_t h_size);


static inline void print_hex(void *buf, size_t len)
{
	unsigned int x;
	char *d = buf;

	fprintf(stderr, "{ ");
	for (x = 0; x < len; x++)
		fprintf(stderr, "%02x, ", d[x]);
	fprintf(stderr, "}");
}

static inline void ht_dump(hashtable_t *ht)
{
	unsigned int i, j;

	fprintf(stderr, "hashtable = {\n");
	for (i = 0; i < HT_SIZE; i++) {
		entry_t *entry = &ht->table[i];

		if (!entry->used)
			continue;

		fprintf(stderr, "  { row = %d, .used = %d, .multi = {\n",
		       i, entry->used);
		for (j = 0; j < entry->used; j++) {
			table_req_t *multi = &entry->multi[j];

			fprintf(stderr, "      { .val = ");
			print_hex(multi, sizeof(*multi));
			fprintf(stderr, " },\n");
		}
		fprintf(stderr, "    },\n"
		       "  },\n");
	}
	fprintf(stderr, "};\n");
}

static inline void table_del_dump(table_del_t *table_del, unsigned int table_del_idx)
{
	unsigned int i;
	table_del_t *t_del;

	fprintf(stderr, "table_del_t table_del[] = {\n");
	fprintf(stderr, "  { .del_txnid =    .ROW_ID=(otid, bucketProperty, rowId) } \n");
	for (i = 0; i < table_del_idx; i++) {
		t_del = &table_del[i];
		fprintf(stderr, "  { .del_txnid = %ld, .ROW_ID=(%ld, %d, %ld) } /* %d. */\n",
		       t_del->del_txnid, t_del->ROW_ID.otid, 
		       t_del->ROW_ID.bucketProperty, t_del->ROW_ID.rowId, i);
	}
	fprintf(stderr, "}; /* table_del_idx=%d\n", table_del_idx);
}

static inline void table_req_dump(table_req_t *table_req, unsigned int table_req_idx)
{
	unsigned int i;
	table_req_t *t_req;

	fprintf(stderr, "table_req_t table_req[] = {\n");
	fprintf(stderr, "  { .ROW_ID=(otid, bucketProperty, rowId) } \n");
	for (i = 0; i < table_req_idx; i++) {
		t_req = &table_req[i];
		fprintf(stderr, "  { .ROW_ID=(%ld, %d, %ld) } /* %d. */\n",
		       t_req->ROW_ID.otid, 
		       t_req->ROW_ID.bucketProperty, t_req->ROW_ID.rowId, i);
	}
	fprintf(stderr, "}; /* table_req_idx=%d\n", table_req_idx);
}

#endif	/* __SNAP_ACID_H__ */
