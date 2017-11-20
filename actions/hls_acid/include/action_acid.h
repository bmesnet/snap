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
#define HT_SIZE     (TABLE_DEL_SIZE * 16) /* size of hashtable - used by MODULO for THE HASH KEY*/
#define HT_MULTI    (TABLE_DEL_SIZE) 	/* multihash entries numbers  */
#define MAX_EXCL_TXN 2          /* maximum number of excluded transactions in exclusion list */
#define MAX_TXNID_NUM 10        /* maximum id number of transactions */

#define REQ_LOW_MARK 9
#define REQ_HIGH_MARK 999
#define DEL_LOW_MARK 999
#define DEL_HIGH_MARK 999
#define DEL_LOW_INPUT 1
#define DEL_HIGH_INPUT 999

typedef enum {false, true} bool;

/* FIXME Make tables entry size a multiple of 64 bytes */
#define ACID_ALIGN 64

//-------- Definition of structures for tables -----
// ROW_ID definition. Used by table_del and table_req
typedef struct ROW_ID_s {
	uint64_t otid;            /*  8 bytes */
	uint32_t bucketProperty;  /*  4 bytes */
	uint64_t rowId;           /*  8 bytes */
} ROW_ID_t;

// table_del is the structure containing all delete transactions
typedef struct table_del_s {
	uint64_t del_txnid;       /*  8 bytes */
	ROW_ID_t ROW_ID;          /* 20 bytes */
	uint8_t reserved[4];      /*  4 bytes */ /* 32B aligned */
} table_del_t;

// table_req is the structure containing all requests
typedef struct table_req_s {
	ROW_ID_t ROW_ID;          /* 20 bytes */
	uint8_t reserved[10];     /* 10 bytes */ /* 32B aligned */
} table_req_t;

// -- hash table --
// entry is the structure used for the hash table
// contains the number of entries used for this hash key
// and the ROW_ID field saved in multi for future comparison
typedef struct entry_s {
	unsigned int used;	/* list entries used */
	table_req_t multi[HT_MULTI];/* fixed size */
} entry_t;

// hashtable definition using entry elements declared previously
typedef struct hashtable_s {
	entry_t table[HT_SIZE];	/* fixed size */
} hashtable_t;

//-------- Parameters used -----------------
//params used for filtering deleted ransactions and requests
typedef struct param_table_s {
	uint64_t excluded_txn_num; /* 8 bytes */
	uint64_t excluded_txn[MAX_EXCL_TXN];   /* MAX_EXCL_TXNID * 8 bytes */
	uint64_t entries;     /* 8 bytes */
	uint64_t low_mark;    /* 8 bytes */
	uint64_t high_mark;   /* 8 bytes */
	uint64_t low_input;   /* 8 bytes */
	uint64_t high_input;  /* 8 bytes */
} param_table_t;

//stucture containing the 1024 bits of results
typedef struct bitset_s {	  /* 1024 bits */
	uint64_t bs_LSB;   	  /*  512 to   0 bits */
	uint64_t bs_MSB; 	  /* 1023 to 512 bits */
} bitset_t;

//--- Data exchanged between Application and action ---
typedef struct acid_job {
	struct snap_addr p_del;     /* 16B - IN: params for table_del:         */
	                            /*     #entries to process, L/H Watermark, L/H Input */
	struct snap_addr t_del;     /* 16B - IN: input table_del for multihash */
	struct snap_addr p_req;     /* 16B - IN: params for table_req:         */
	                            /*     #entries to process, L/H Watermark, L/H Input    */
	struct snap_addr t_req;     /* 16B - IN: input table_req containing req to search */
	struct snap_addr bs;        /* 16B - OUT: out 1024 bits containing results of search */
	struct snap_addr hashtable; /* 16B - CACHE: multihash table */
} acid_job_t; 	// 108 Bytes or less (12B remaining)

#ifdef __cplusplus
}
#endif

#endif	/* __ACTION_ACID_H__ */
