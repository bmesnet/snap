Example of ACID deletion lookup algorithm
source code files are located in:
- sw/main_acid.c  -- used only in standalone mode
- sw/snap_acid.c  -- called by main_acid or by JNICall
- sw/snap_acid.h 
- sw/action_acid.c
- include/action_acid.h -- global parameters
- jni/JNICall.c  -- called by the acid benchmark

compile with make from this directory /sw

execute in standalone mode with 
`SNAP_CONFIG=CPU ./main_acid`
`SNAP_CONFIG=CPU ./main_acid -v -D50 -R60` 
`	-D50 => to create 50 del_txnid to build the hash table`
`	-R50 => to create 50 request to check if they are part of the hash table`

execute called from the acid benchmark 

======================================
Parameters in include/action_acid.h:
`#define TABLE_DEL_SIZE 32       // number of del_txnid + ROW_ID entries` 
`   => allows to fill the hash table by bursts`
`#define TABLE_REQ_SIZE 32       // number of ROW_ID requests` 
`   => typically set to 1024 as for looking for 1024 requests` 
`#define HT_SIZE     (TABLE_DEL_SIZE * 16) // size of hashtable` 
`   => size of the hashtable => 4GB ?`
`#define HT_MULTI    (TABLE_DEL_SIZE)    // multihash entries numbers` 
`   => changing the HT_MULTI value to a small value (2) highlights collisions management`

NOTE : overall time reported includes printf used for debug
