#include <string.h>
#include <stdio.h>
#include "org_apache_hadoop_hive_ql_io_orc_JNICall.h"

#include <libsnap.h>
#include <snap_tools.h>
#include <snap_s_regs.h>
#include <snap_acid.h>
#include <libcxl.h>

char card_allocated = 0;
hashtable_t hashtable __attribute__((aligned(64)));

int capi_init ()
{
	uint64_t ret;
	char cxl_device [64];
	unsigned int seed = 1967;

	srand(seed);

	printf("\n (key used is ROW_ID.row_id modulo %d (HT_SIZE))", (int)HT_SIZE); //32*16);

	snap_action_flag_t action_irq = (SNAP_ACTION_DONE_IRQ | SNAP_ATTACH_IRQ);
 	unsigned int timeout = 10;
	int card_no = 0;

        ////////////////////////////////////////////
        //Card No., timeout, interrupt has to be set.
        //If openings() fails, it exits before malloc any arrays.
        ret = openings(card_no, timeout, action_irq);
	card_allocated = 1;
	printf("\n[capi_init] card_allocated = %d\n", card_allocated);
	return ret;
}        
 
JNIEXPORT jint JNICALL Java_org_apache_hadoop_hive_ql_io_orc_JNICall_PrintfakeDeleteDeltas
	(JNIEnv *env, jobject obj, jobject sortMerger) {
 
        int count = 0;

        jclass iter_class = (*env)->GetObjectClass(env, sortMerger);

        // get some methods
        jmethodID hasNext = (*env)->GetMethodID(env, iter_class, "hasNext", "()Z");
        jmethodID next = (*env)->GetMethodID(env, iter_class, "next", "()Ljava/lang/Object;"); 
        //jmethodID next = (*env)->GetMethodID(env, iter_class, "next", "()Ljava/util/Map/Entry;");

        while ((*env)->CallBooleanMethod(env, sortMerger, hasNext)) {
           count++;
           jobject entry = (*env)->CallObjectMethod(env, sortMerger, next);
	   printf("count=%d - entry=%ld\n", count, entry);
        }

	//int i, sum = 0;
	return count;
}

JNIEXPORT jint JNICALL Java_org_apache_hadoop_hive_ql_io_orc_JNICall_SnapInitHashTable
        (JNIEnv *env, jobject obj){

	static table_del_t t_del[TABLE_DEL_SIZE] __attribute__((aligned(ACID_ALIGN)));
	static table_req_t t_req[TABLE_REQ_SIZE] __attribute__((aligned(ACID_ALIGN)));
	//static hashtable_t hashtable __attribute__((aligned(64)));
	static bitset_t bs __attribute__((aligned(128)));
        unsigned int t_del_entries = 1; // for this JNI example, we are enrterig the delete entries one by one
        unsigned int t_req_entries = 0; // No request entries in this function
	unsigned int t_del_tocopy = 0;
	unsigned int t_req_tocopy = 0;
	param_table_t p_req_t;
        param_table_t p_del_t;
        p_req_t.entries = 0;
        t_req_tocopy = 0;

	if (card_allocated == 0) {
		if (capi_init ())
			return -1;

	   printf("\n[hashinit] Init of the hash table \n");
           p_del_t.entries = UINT_MAX;
           snap_acid( &p_del_t, sizeof(param_table_t),
                t_del, t_del_tocopy * sizeof(table_del_t),
                &p_req_t, sizeof(param_table_t),
                t_req, t_req_tocopy * sizeof(table_req_t),
                &bs, sizeof(bs),
                &hashtable, sizeof(hashtable));
	   }
	}

JNIEXPORT jint JNICALL Java_org_apache_hadoop_hive_ql_io_orc_JNICall_SnapBuildHashTable
        (JNIEnv *env, jobject obj, jint index, jobject key){

        jclass key_class = (*env)->GetObjectClass(env, key);
        jfieldID rowId = (*env)->GetFieldID(env, key_class, "rowId", "J");
        jfieldID originalTransactionId = (*env)->GetFieldID(env, key_class, "originalTransactionId", "J");
        jfieldID  bucketProperty = (*env)->GetFieldID(env, key_class, "bucketProperty", "I");

        /** GET ATTIBUTES OF DeleteRecordKey JAVA OBJECT **/
        jlong rowid_value = (*env)->GetLongField(env, key, rowId);
        jlong otid_value  = (*env)->GetLongField(env, key, originalTransactionId);
        jint bucketProperty_value = (*env)->GetLongField(env, key, bucketProperty);
        //printf("\n[JNICall.c]********** index:%d - otid=%ld - Prop=%d - Row=%ld ***\n", 
	//	index, otid_value, bucketProperty_value, rowid_value);

	/// START -- Inserting call to snap fucntion
	static table_del_t t_del[TABLE_DEL_SIZE] __attribute__((aligned(ACID_ALIGN)));
	static table_req_t t_req[TABLE_REQ_SIZE] __attribute__((aligned(ACID_ALIGN)));
	//static hashtable_t hashtable __attribute__((aligned(64)));
	static bitset_t bs __attribute__((aligned(128)));
	
        unsigned int t_del_entries = 1; // for this JNI example, we are enrterig the delete entries one by one
        unsigned int t_req_entries = 0; // No request entries in this function
	unsigned int t_del_tocopy = 0;
	unsigned int t_req_tocopy = 0;
	param_table_t p_req_t;
        param_table_t p_del_t;

	//printf("\n[JNICall.c]>> BUILDING THE HASH TABLE FOR %d ENTRIES (bursts of %d entries)\n",
	//	t_del_entries, TABLE_DEL_SIZE);
	//fill he params structure
        p_req_t.entries = 0;
        t_req_tocopy = 0;

        p_del_t.entries = t_del_entries;
        p_del_t.low_mark = DEL_LOW_MARK;
        p_del_t.high_mark = DEL_HIGH_MARK;
        p_del_t.low_input = DEL_LOW_INPUT;
        p_del_t.high_input = DEL_HIGH_INPUT;
        //printf("[JNICall.c]>> Parameters used are: LOW-MARK=%d HIGH-MARK=%d LOW-INPUT=%d HIGH-INPUT=%d\n",
         //       DEL_LOW_MARK, DEL_HIGH_MARK, DEL_LOW_INPUT, DEL_HIGH_INPUT);

	t_del_tocopy = MIN(ARRAY_SIZE(t_del), t_del_entries);
        t_del[0].del_txnid = rand() %1000;  // FIXME o be change
        t_del[0].ROW_ID.otid = otid_value;
        t_del[0].ROW_ID.bucketProperty = bucketProperty_value;
        t_del[0].ROW_ID.rowId = rowid_value;

        snap_acid( &p_del_t, sizeof(param_table_t),
                t_del, t_del_tocopy * sizeof(table_del_t),
                &p_req_t, sizeof(param_table_t),
                t_req, t_req_tocopy * sizeof(table_req_t),
                &bs, sizeof(bs),
                &hashtable, sizeof(hashtable));

	// END -- Inserting call to snap fucntion
	
        //printf("********** sizeof key=%d -otid=%d - Prop=%d - Row=%d ***\n", 
	//	sizeof(key), sizeof(originalTransactionId), sizeof(bucketProperty), sizeof(rowId));
        //printf("********** sizeof key=%d -otid=%d - Prop=%d - Row=%d ***\n", 
	//	sizeof(key), sizeof(otid_value), sizeof(bucketProperty_value), sizeof(rowid_value));

        /* try to find java object memory address */
        jobject myobj = (*env)->NewGlobalRef(env, key);
        //printf("\n[JNICall.c]test memory address key=0x%llx \n", myobj);

	/* line below is mandatory so that garbage collector can clean the object */
        (*env)->DeleteGlobalRef(env, myobj);

}



JNIEXPORT jint JNICALL Java_org_apache_hadoop_hive_ql_io_orc_JNICall_SubSnapReadHashTable
        (JNIEnv *env, jobject obj, jobject selectedBitSet, jlongArray originalTransactionVector, jlongArray bucketProperties, jlongArray rowIdVector){


        jclass bitset_class = (*env)->GetObjectClass(env, selectedBitSet);
        jmethodID toByteArray = (*env)->GetMethodID(env, bitset_class, "toByteArray", "()[B");
        jmethodID setBitToBitset = (*env)->GetMethodID(env, bitset_class, "set", "(IZ)V");
        jmethodID bitsetSize = (*env)->GetMethodID(env, bitset_class, "size", "()I");
        jmethodID clear = (*env)->GetMethodID(env, bitset_class, "clear", "(I)V");
        jmethodID clearall = (*env)->GetMethodID(env, bitset_class, "clear", "()V");
        jmethodID valueOf = (*env)->GetStaticMethodID(env, bitset_class, "valueOf", "([B)Ljava/util/BitSet;");
        jmethodID or = (*env)->GetMethodID(env, bitset_class, "or", "(Ljava/util/BitSet;)V");
        jbyteArray arr = (jbyteArray) (*env)->CallObjectMethod(env, selectedBitSet, toByteArray);
 

 
	/// START -- Inserting call to snap fucntion
	static table_del_t t_del[TABLE_DEL_SIZE] __attribute__((aligned(ACID_ALIGN)));
	static table_req_t t_req[TABLE_REQ_SIZE] __attribute__((aligned(ACID_ALIGN)));
	//static hashtable_t hashtable __attribute__((aligned(64)));
	static bitset_t bs __attribute__((aligned(128)));
	
        unsigned int t_del_entries = 0; // No delete entry in this function
        unsigned int t_req_entries; // for this JNI example, we are enrterig the requested 1024 by 1024
	unsigned int t_del_tocopy = 0;
	unsigned int t_req_tocopy = 0;
	param_table_t p_req_t;
        param_table_t p_del_t;

        if(originalTransactionVector == NULL || bucketProperties == NULL || rowIdVector ==NULL){
           printf("Error: one of the vector is null\n");
        }else{
           jsize len_a = (*env)->GetArrayLength(env, originalTransactionVector);
           jsize len_b = (*env)->GetArrayLength(env, bucketProperties);
           jsize len_c = (*env)->GetArrayLength(env, rowIdVector);

	   t_req_entries = len_a;

           jlong *otids = (*env)->GetLongArrayElements(env, originalTransactionVector, 0);
           jlong *buckids = (*env)->GetLongArrayElements(env, bucketProperties, 0);
           jlong *rowids = (*env)->GetLongArrayElements(env, rowIdVector, 0);

           //printf("bitset before processing = %ld\n",bitset_value);

           int i=0;
           //for (i=0; i<len_a; i++) {
           //    printf("i = %ld | ORIGINAL Tx ID = %ld | BUCKET PROP = %ld | ROW ID = %ld | length = %ld\n",
           //        i, otids[i], buckids[i], rowids[i], len_a);
           //}

	//printf("\n[JNICall.c]>> READING THE HASH TABLE FOR %d ENTRIES (bursts of %d entries)\n",
	//	t_req_entries, TABLE_REQ_SIZE);
	//fill he params structure
        p_del_t.entries = 0;
        t_del_tocopy = 0;

        p_req_t.entries = t_req_entries;
        p_req_t.low_mark = REQ_LOW_MARK;
        p_req_t.high_mark = REQ_HIGH_MARK;
        p_req_t.low_input = REQ_LOW_INPUT;
        p_req_t.high_input = REQ_HIGH_INPUT;
        //printf("[JNICall.c]>> Parameters used are: LOW-MARK=%d HIGH-MARK=%d LOW-INPUT=%d HIGH-INPUT=%d\n",
         //       REQ_LOW_MARK, REQ_HIGH_MARK, REQ_LOW_INPUT, REQ_HIGH_INPUT);

	t_req_tocopy = MIN(ARRAY_SIZE(t_req), t_req_entries);
	// FIXME : this can certainly be declared a better way to copy w/o this loop
        for (i=0; i<len_a; i++) {
        	t_req[i].ROW_ID.otid = otids[i];
        	t_req[i].ROW_ID.bucketProperty = buckids[i];
        	t_req[i].ROW_ID.rowId = rowids[i];
	}

        snap_acid( &p_del_t, sizeof(param_table_t),
                t_del, t_del_tocopy * sizeof(table_del_t),
                &p_req_t, sizeof(param_table_t),
                t_req, t_req_tocopy * sizeof(table_req_t),
                &bs, sizeof(bs),
                &hashtable, sizeof(hashtable));

                //printf("\nBitset dump:\n      Byte |1-0|-|3-2|-[5-4|..... \n");
                //__hexdump(stdout, &bs, sizeof(bs));


         // WORK IN PROGRESS
         jbyteArray array = (*env)->NewByteArray(env, sizeof(bs));
         (*env)->SetByteArrayRegion(env, array, 0, sizeof(bs), (jbyte*)&bs);

         jobject newBitset = (*env)->CallStaticObjectMethod(env, selectedBitSet, valueOf, array); 
         
         // clean all then apply or operator (maybe smarter improvement here
         (*env)->CallVoidMethod(env, selectedBitSet, clearall);
         (*env)->CallVoidMethod(env, selectedBitSet, or, newBitset); 



        }
}


 
void main(){}
