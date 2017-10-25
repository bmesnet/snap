#ifndef __ACTION_ACID_H__
#define __ACTION_ACID_H__

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

#include <snap_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ACID_ACTION_TYPE     0x10141002

#define TABLE_DEL_SIZE 32 	/* size of the bursts - nb of del_txnid  entries */
#define TABLE_REQ_SIZE 32 	/* size of the bursts - nb of ROW_ID requests */
#define HT_SIZE     (TABLE_DEL_SIZE * 16) /* size of hashtable */
#define HT_MULTI    (TABLE_DEL_SIZE) 	/* multihash entries numbers  */

#define LAST_TXNID_ELMT ULONG_MAX /* FIXME find the right value for that */

//typedef char hashdata_t[256];



/* FIXME Make tables entry size a multiple of 64 bytes */
#define ACID_ALIGN 64

typedef struct ROW_ID_s {
	uint64_t otid;            /*  8 bytes */
	uint32_t bucketProperty;  /*  4 bytes */
	uint64_t rowId;           /*  8 bytes */
} ROW_ID_t;

typedef struct table_del_s {
	uint64_t del_txnid;       /*  8 bytes */
	ROW_ID_t ROW_ID;          /* 20 bytes */
	uint8_t reserved[4];      /*  4 bytes */ /* 32B aligned */
} table_del_t;

typedef struct table_req_s {
	ROW_ID_t ROW_ID;          /* 20 bytes */
	uint8_t reserved[10];     /* 10 bytes */ /* 32B aligned */
} table_req_t;

typedef struct entry_s {
	unsigned int used;	/* list entries used */
	table_req_t multi[HT_MULTI];/* fixed size */
} entry_t;

typedef struct bitset_s {	  /* 1024 bits */
	uint64_t bs_LSB;   	  /*  512 to   0 bits */
	uint64_t bs_MSB; 	  /* 1023 to 512 bits */
} bitset_t;

typedef struct hashtable_s {
	entry_t table[HT_SIZE];	/* fixed size */
} hashtable_t;

typedef struct acid_job {
	struct snap_addr t_del; /* IN: input table_del for multihash */
	struct snap_addr t_req; /* IN: input table_req containing req to search */
	struct snap_addr bs; /* OUT: out 1024 bits containing results of search */
	struct snap_addr hashtable; /* CACHE: multihash table */

	uint64_t t_del_processed; /* #entries to process */
	uint64_t t_req_processed; /* #entries to process */
} acid_job_t;

#ifdef __cplusplus
}
#endif

#endif	/* __ACTION_ACID_H__ */
