#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sim.h"
#include <math.h>

//Global counter inits:

//L1 global counters
int reads_l1=0, writes_l1=0, read_hits_l1=0, write_hits_l1=0, read_misses_l1=0, write_misses_l1=0, writebacks_l1=0; 
int lru_counter_l1=0, index_len_l1=0, bo_len_l1=0,tag_len_l1=0, tracker_l1=0;
float miss_rate_l1=0.0;
uint32_t mask_for_index_l1;
uint32_t index_l1;
uint32_t bo_l1;
uint32_t tag_l1;
int prefetches_l1 = 0, prefetches_l2 = 0,prefetch_reads_l2=0, prefetch_read_misses_l2=0;

//L2 global counters
int reads_l2=0, writes_l2=0, read_hits_l2=0, write_hits_l2=0, read_misses_l2=0, write_misses_l2=0, writebacks_l2=0; 
int lru_counter_l2=0, index_len_l2=0, bo_len_l2=0,tag_len_l2=0, tracker_l2=0;
float miss_rate_l2=0.0;
uint32_t mask_for_index_l2;
uint32_t index_l2;
uint32_t bo_l2;
uint32_t tag_l2;

int level_select;
uint32_t print_addr;
int number_of_measurements = 17;
int pref_lru_count = 0;
pref_params *pref;
uint32_t **prefetcher;
int total_mem_traffic=0;
int pref_hit =0, pref_miss =0;
int i,j;

/*  "argc" holds the number of command-line arguments.
    "argv[]" holds the arguments themselves.

    Example:
    ./sim 32 8192 4 262144 8 3 10 gcc_trace.txt
    argc = 9
    argv[0] = "./sim"
    argv[1] = "32"
    argv[2] = "8192"
    ... and so on
*/
int main (int argc, char *argv[]) {
   FILE *fp;			// File pointer.
   char *trace_file;		// This variable holds the trace file name.
   cache_params_t params;	// Look at the sim.h header file for the definition of struct cache_params_t.
   char rw;			// This variable holds the request's type (read or write) obtained from the trace.
   uint32_t addr;		// This variable holds the request's address obtained from the trace.
				// The header file <inttypes.h> above defines signed and unsigned integers of various sizes in a machine-agnostic way.  "uint32_t" is an unsigned integer of 32 bits.

   // Exit with an error if the number of command-line arguments is incorrect.
   if (argc != 9) {
      printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
      exit(EXIT_FAILURE);
   }
    
   // "atoi()" (included by <stdlib.h>) converts a string (char *) to an integer (int).
   params.BLOCKSIZE = (uint32_t) atoi(argv[1]);
   params.L1_SIZE   = (uint32_t) atoi(argv[2]);
   params.L1_ASSOC  = (uint32_t) atoi(argv[3]);
   params.L2_SIZE   = (uint32_t) atoi(argv[4]);
   params.L2_ASSOC  = (uint32_t) atoi(argv[5]);
   params.PREF_N    = (uint32_t) atoi(argv[6]);
   params.PREF_M    = (uint32_t) atoi(argv[7]);
   trace_file       = argv[8];

   // Open the trace file for reading.
   fp = fopen(trace_file, "r");
   if (fp == (FILE *) NULL) {
      // Exit with an error if file open failed.
      printf("Error: Unable to open file %s\n", trace_file);
      exit(EXIT_FAILURE);
   }
    
   // Print simulator configuration.
   printf("===== Simulator configuration =====\n");
   printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
   printf("L1_SIZE:    %u\n", params.L1_SIZE);
   printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
   printf("L2_SIZE:    %u\n", params.L2_SIZE);
   printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
   printf("PREF_N:     %u\n", params.PREF_N);
   printf("PREF_M:     %u\n", params.PREF_M);
   printf("trace_file: %s\n", trace_file);
    //printf("===================================\n");
    printf("\n");
    int i,j;
    // Read requests from the trace file and echo them back.
    //Prefetcher
    if(params.PREF_N!=0){
    pref =(pref_params*)malloc(params.PREF_N*sizeof(pref_params));
    prefetcher = (uint32_t **) malloc(params.PREF_M*sizeof(uint32_t *));
    for(i=0;i<params.PREF_M;i++){
        prefetcher[i] = (uint32_t *)malloc(params.PREF_N*sizeof(uint32_t));
    }
    }
    else{
        pref =(pref_params*)malloc(1*sizeof(pref_params));
        prefetcher = (uint32_t **) malloc(1*sizeof(uint32_t *));
        for(i=0;i<1;i++){
        prefetcher[i] = (uint32_t *)malloc(1*sizeof(uint32_t));
    }
    }

    for(j=0; j<params.PREF_N;j++){
        pref[j].index = j;
        pref[j].valid = 0;
        pref[j].lru_count = 0;
    }
    for(i=0;i<params.PREF_M;i++){
        for(j=0; j<params.PREF_N;j++){
            prefetcher[i][j]=0;         
        }
    }

    
    // Cache Initialization:

    //L1 starts here:
    int sets_l1 = (params.L1_SIZE)/((params.L1_ASSOC)*(params.BLOCKSIZE));
    //Allocating memory
    cache_blocks **cache_l1 = (cache_blocks**) malloc((sets_l1) * sizeof(cache_blocks*));
    for(i=0;i<sets_l1;i++){
        cache_l1[i] = (cache_blocks*) malloc((params.L1_ASSOC) * sizeof(cache_blocks));
    }
    
    //Creating another cache block to print
    cache_blocks *cache_l1_print = (cache_blocks*) malloc((params.L1_ASSOC) * sizeof(cache_blocks));
    cache_blocks *cache_l2_print = (cache_blocks*) malloc((params.L2_ASSOC) * sizeof(cache_blocks));
    //Getting lenghts of Tag, Index and Block Offset
    index_len_l1 =(int)log2(sets_l1);
    bo_len_l1 =(int)log2(params.BLOCKSIZE);
    tag_len_l1 = 32 - (index_len_l1+bo_len_l1);   
    //Initializing cache
    for(i=0;i<sets_l1;i++){
        for(j=0;j<params.L1_ASSOC;j++){
            cache_l1[i][j].valid = 0;
            cache_l1[i][j].dirty = 0;
            cache_l1[i][j].tag = 0;
            cache_l1[i][j].lru_count = 0;
        }
    }

    //L2 starts here:
    int sets_l2=0;  
    if(params.L2_SIZE != 0) 
        sets_l2 = (params.L2_SIZE)/((params.L2_ASSOC)*(params.BLOCKSIZE));
     cache_blocks **cache_l2 = (cache_blocks**) malloc((sets_l2) * sizeof(cache_blocks*));
     for(i=0;i<sets_l2;i++){
         cache_l2[i] = (cache_blocks*) malloc((params.L2_ASSOC) * sizeof(cache_blocks));
     }
     if(params.L2_SIZE != 0){   
         index_len_l2 =(int)log2(sets_l2);
         bo_len_l2 = (int)log2(params.BLOCKSIZE);
         tag_len_l2 = 32 - (index_len_l2+bo_len_l2);    
         for(i=0;i<sets_l2;i++){
             for(j=0;j<params.L2_ASSOC;j++){
                 cache_l2[i][j].valid = 0;
                 cache_l2[i][j].dirty = 0;
                 cache_l2[i][j].tag = 0;
                 cache_l2[i][j].lru_count = 0;
             }
         }  
     }
     while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {    // Stay in the loop if fscanf() successfully parsed two tokens as specified.
    //     if (rw == 'r')
    //         printf("r %x\n", addr);
    //     else if (rw == 'w')
    //         printf("w %x\n", addr);
    //     else {
    //         printf("Error: Unknown request type %c.\n", rw);
 	//         exit(EXIT_FAILURE);
    //    }
      print_addr = addr;
       //decoding addresses for L1 here:
       mask_for_index_l1 = (1 << bo_len_l1) - 1;
       index_l1 = (addr<<tag_len_l1)>>(tag_len_l1+bo_len_l1);
       if(sets_l1 == 1)
            index_l1 =0;
       bo_l1 = addr & mask_for_index_l1;
       tag_l1 = (addr>>(index_len_l1+bo_len_l1));

       //decoding addresses for L2 here:
       mask_for_index_l2 = (1 << bo_len_l2) - 1;
       index_l2 = (addr<<tag_len_l2)>>(tag_len_l2+bo_len_l2);
       if(sets_l2 == 1)
            index_l2 =0;
       bo_l2 = addr & mask_for_index_l2;
       tag_l2 = (addr>>(index_len_l2+bo_len_l2));

        if(rw == 'r'){
            read_l1(cache_l1, cache_l2, params);
        }
        else if(rw == 'w'){
            write_l1(cache_l1, cache_l2, params);
        }
    }
    miss_rate_l1 = (float)(((float)read_misses_l1+(float)write_misses_l1)/((float)reads_l1+(float)writes_l1));
    
    if(params.L2_SIZE == 0){
	    miss_rate_l2 = 0;
    }
    else{
	    miss_rate_l2=(float)(((float)read_misses_l2+(float)write_misses_l2)/((float)reads_l2));
    }
    
    if(params.L2_SIZE == 0){
            total_mem_traffic=read_misses_l1+write_misses_l1+writebacks_l1+prefetches_l1;
    }
    else if(params.L2_SIZE != 0){
        total_mem_traffic=read_misses_l2+write_misses_l2+writebacks_l2+prefetches_l2;
    }
    else{
            total_mem_traffic=read_misses_l2+write_misses_l2+writebacks_l2;
    }

    //To print L1 trace:
    printf(" ===== L1 Contents =====\n");
    //printf("\n");
    for(i=0;i<sets_l1;i++){
	for(j=0;j<params.L1_ASSOC;j++){
		cache_l1_print[j] = cache_l1[i][j];
	}
        printf("set      %d: ", i);
 	level_select=1;
	
	array_arrange(cache_l1_print,params);
	for(j=0; j<params.L1_ASSOC; j++){
		printf("\t%x",cache_l1_print[j].tag);
		if(cache_l1_print[j].dirty == 1){
			printf(" D");
		}
	}
	printf("\n");
    }   
    printf("\n");	
    
    //To print L2 trace:
    if(params.L2_SIZE != 0){
        printf(" ===== L2 Contents =====\n");
        for(i=0;i<sets_l2;i++){
            for(j=0;j<params.L2_ASSOC;j++){
                cache_l2_print[j] = cache_l2[i][j];
            }
            printf("set      %d: ", i);
            level_select=2;
            array_arrange(cache_l2_print,params);
            for(j=0; j<params.L2_ASSOC; j++){
                printf("\t%x",cache_l2_print[j].tag);
                if(cache_l2_print[j].dirty == 1){
                    printf(" D");
                }
            }
            printf("\n");
        }
            printf("\n");
    }

    if(params.PREF_N!=0){
        printf(" ===== Stream Buffer(s) contents =====\n");
        for(i =0; i< params.PREF_N; i++){
            for(j = 0; j<params.PREF_M; j++){
                printf(" %x ", prefetcher[j][pref[i].index]);
            }
            printf("\n");
        }
        printf("\n");
    }
    
    
    //To print measurements
    //============================================================
    printf("===== Measurements =====\n");
    printf("a. L1 reads: %d\n", reads_l1);
    printf("b. L1 read misses: %d\n", read_misses_l1);
    printf("c. L1 writes: %d\n", writes_l1);
    printf("d. L1 write misses: %d\n", write_misses_l1);
    printf("e. L1 miss rate: %.4f\n", miss_rate_l1);
    printf("f. L1 writebacks: %d\n", writebacks_l1);
    printf("g. L1 prefetches: %d\n", prefetches_l1);
    printf("h. L2 reads (demand): %d\n", reads_l2);
    printf("i. L2 read misses (demand): %d\n", read_misses_l2);
    printf("j. L2 reads (prefetch): %d\n", prefetch_reads_l2);
    printf("k. L2 read misses (prefetch): %d\n", prefetch_read_misses_l2);
    printf("l. L2 writes: %d\n", writes_l2);
    printf("m. L2 write misses: %d\n", write_misses_l2);
    printf("n. L2 miss rate: %.4f\n", miss_rate_l2);
    printf("o. L2 writebacks: %d\n", writebacks_l2);
    printf("p. L2 prefetches: %d\n", prefetches_l2);
    printf("q. memory traffic: %d\n", total_mem_traffic);
    //============================================================

    for(i=0;i<sets_l1;i++){
            free(cache_l1[i]);
        }
    free(cache_l1);

    for(i=0;i<sets_l2;i++){
            free(cache_l2[i]);
        }
    free(cache_l2);
    
    free(pref);
    if(params.PREF_N!=0){
    for(i=0;i<params.PREF_M;i++){
            free(prefetcher[i]);
    }
    }
    else{
        for(i=0;i<1;i++){
            free(prefetcher[i]);
    }
    }
    free(prefetcher);

    return(0);
}

void read_l1(cache_blocks **cache_1, cache_blocks **cache_2, cache_params_t param){
    reads_l1++;
    int flag_hit=0,flag_evict=0,to_evict_l1=0;
    tracker_l1+=1;

    //Checking if it is a read hit
    for(i=0;i<param.L1_ASSOC;i++){
        if(cache_1[index_l1][i].tag == tag_l1 && cache_1[index_l1][i].valid == 1){
	        read_hits_l1++;
	        flag_hit = 1;
	        cache_1[index_l1][i].lru_count = tracker_l1;
            break;
	    }
    }
    uint32_t temp = (tag_l1 << index_len_l1) | index_l1;
    if(param.L2_SIZE ==0){
        level_select =1;
        if(param.PREF_N !=0)
            pref_hit_miss(param, temp, flag_hit);
        level_select = 0;
    }
    //Handling read misses
    if(flag_hit == 0){
	    //read_misses_l1++;
        if(pref_miss==1 && param.PREF_N!=0 && param.L2_SIZE ==0)
            read_misses_l1++;
        else if(param.PREF_N == 0 && param.L2_SIZE ==0)
            read_misses_l1++;
        else if(param.L2_SIZE != 0)
            read_misses_l1++;	
	    for(j=0;j<param.L1_ASSOC;j++){
	        if(cache_1[index_l1][j].valid == 0){
		        to_evict_l1=j;
		        break;
	        }
	        if(cache_1[index_l1][j].valid == 1){
		        flag_evict++;
	        }
	    }   
	    if(flag_evict == param.L1_ASSOC){
            level_select = 1;
	        to_evict_l1 =lru_update(cache_1, index_l1, param); 
            level_select = 0;
	    }
	    if(cache_1[index_l1][to_evict_l1].valid == 1 && cache_1[index_l1][to_evict_l1].dirty == 1){
	        writebacks_l1++;
	        if(param.L2_SIZE != 0){
	            write_l2(cache_1, cache_2, to_evict_l1, param);
	        }	    
	    }
	    if(param.L2_SIZE != 0){
	        read_l2(cache_2, param);
	    }
	    cache_1[index_l1][to_evict_l1].valid = 1;
	    cache_1[index_l1][to_evict_l1].dirty = 0;
	    cache_1[index_l1][to_evict_l1].set_nu = index_l1;
	    cache_1[index_l1][to_evict_l1].tag = tag_l1;
	    cache_1[index_l1][to_evict_l1].lru_count = tracker_l1;
    }
    pref_hit =0;
    pref_miss =0;
}
void read_l2(cache_blocks **cache_2, cache_params_t param){

    reads_l2++;
    int flag_hit=0, flag_evict=0, to_evict_l1=0;
    tracker_l2+=1;

    //Checking for read hits
    for(i=0;i<param.L2_ASSOC;i++){
        if(cache_2[index_l2][i].valid == 1 && cache_2[index_l2][i].tag == tag_l2){
	        read_hits_l2++;
	        flag_hit = 1;
	        cache_2[index_l2][i].lru_count = tracker_l2;
	        break;
	    }
    }
    level_select = 2;
    uint32_t temp = (tag_l2 << index_len_l2) | index_l2;
    if(param.PREF_N !=0)
        pref_hit_miss(param, temp, flag_hit);
    if(pref_miss==1 && param.PREF_N!=0)
        read_misses_l2++;
    level_select = 0;
    //Handling read misses
    if(flag_hit == 0){
        if(param.PREF_N == 0)
            read_misses_l2++;
	    for(i=0;i<param.L2_ASSOC;i++){
	        if(cache_2[index_l2][i].valid == 0){
		        to_evict_l1 = i;
		        break;
	        }
	        if(cache_2[index_l2][i].valid == 1){
		        flag_evict++;
	        }
	    }
	    if(flag_evict == param.L2_ASSOC){
            level_select = 2;
	        to_evict_l1 = lru_update(cache_2, index_l2, param);
            level_select = 0;
	    }
	    if(cache_2[index_l2][to_evict_l1].valid == 1 && cache_2[index_l2][to_evict_l1].dirty == 1){
	        writebacks_l2++;
	    }
	    cache_2[index_l2][to_evict_l1].valid = 1;
	    cache_2[index_l2][to_evict_l1].dirty = 0;
	    cache_2[index_l2][to_evict_l1].tag = tag_l2;
	    cache_2[index_l2][to_evict_l1].set_nu = index_l2;
	    cache_2[index_l2][to_evict_l1].lru_count = tracker_l2;
    }
    pref_hit =0;
    pref_miss =0;
}

int lru_update(cache_blocks **cache, uint32_t index, cache_params_t param){
    int i,j;
    int lru=0, store_way=0;
    //LRU updating for L2
    if(level_select == 2){
        //Handling LRU detection
        lru = cache[index][0].lru_count;
        for(i=1;i<param.L2_ASSOC;i++){
	        if(cache[index_l2][i].lru_count < lru){
	            lru = cache[index_l2][i].lru_count;
	            store_way = i;
	        }
        }
    }
    //LRU updating for L1
    else if(level_select == 1){
        //Handling LRU detection
        lru = cache[index_l1][0].lru_count;
        for(i=1;i<param.L1_ASSOC;i++){
            if(cache[index_l1][i].lru_count < lru){
	            lru = cache[index_l1][i].lru_count;
	            store_way = i;
	        }
        }
    }
    return store_way;
}

void write_l1(cache_blocks **cache_1, cache_blocks **cache_2, cache_params_t param){
    int i,j;
    writes_l1++;
    int flag_hit=0, flag_evict=0, to_evict_l1=0;
    tracker_l1+=1;
    
    //Checking for write hits
    for(i=0;i<param.L1_ASSOC;i++){
        if(cache_1[index_l1][i].tag == tag_l1 && cache_1[index_l1][i].valid == 1){
	        write_hits_l1++;
	        flag_hit = 1;
	        cache_1[index_l1][i].dirty = 1;
	        cache_1[index_l1][i].lru_count = tracker_l1;
	        break;
	    }
    }
    uint32_t temp = (tag_l1 << index_len_l1) | index_l1;
    if(param.L2_SIZE ==0){
        level_select =1;
        if(param.PREF_N !=0)
            pref_hit_miss(param, temp, flag_hit);
        if(pref_miss)
            write_misses_l1++;
        level_select =0;
    }
    //Handling write misses
    if(flag_hit == 0){
        if(param.PREF_N == 0 && param.L2_SIZE ==0)
            write_misses_l1++;
        else if(param.L2_SIZE != 0)
            write_misses_l1++;
	    for(i=0;i<param.L1_ASSOC;i++){
	        if(cache_1[index_l1][i].valid == 0){   
		        to_evict_l1 = i;
		        break;
	        }
	        else{
		        flag_evict++;
	        }
        }
	    if(flag_evict == param.L1_ASSOC){
            level_select = 1;
	        to_evict_l1 = lru_update(cache_1, index_l1, param);
            level_select = 0;
	    }
	    if(cache_1[index_l1][to_evict_l1].valid == 1 && cache_1[index_l1][to_evict_l1].dirty == 1){
	        writebacks_l1++;
	        if(param.L2_SIZE!= 0){
	            write_l2(cache_1, cache_2, to_evict_l1, param);
	        }
	    }
	    if(param.L2_SIZE != 0){
	        read_l2(cache_2, param);
	    }
	    cache_1[index_l1][to_evict_l1].valid = 1;
	    cache_1[index_l1][to_evict_l1].dirty = 1;
	    cache_1[index_l1][to_evict_l1].set_nu = index_l1;
	    cache_1[index_l1][to_evict_l1].tag = tag_l1;
	    cache_1[index_l1][to_evict_l1].lru_count = tracker_l1 ;
    }
    pref_hit =0;
    pref_miss =0;
}

void write_l2(cache_blocks **cache_1, cache_blocks **cache_2, int to_evict, cache_params_t param){
    int i,j;

    writes_l2++;

    uint32_t convert=0, convert_tag_l1=0, convert_tag_l2=0;
    uint32_t convert_index_l2=0;

    //To get the L2 address from L1 tag
    convert_tag_l1 = cache_1[index_l1][to_evict].tag;
    convert = ((((convert_tag_l1<<index_len_l1) | (index_l1)) << (bo_len_l1)) | (bo_l1));
    convert_tag_l2 = convert>>(index_len_l2+bo_len_l2);
    convert_index_l2 = (convert<<tag_len_l2)>>(tag_len_l2+bo_len_l2);

    int flag_hit=0, flag_evict=0, to_evict_l2=0;
    tracker_l2+=1;

    //Checking for write hits
    for(i=0;i<param.L2_ASSOC;i++){
	    if(cache_2[convert_index_l2][i].valid == 1 && cache_2[convert_index_l2][i].tag == convert_tag_l2){
	        write_hits_l2++;
	        flag_hit = 1;
	        cache_2[convert_index_l2][i].dirty = 1;
	        cache_2[convert_index_l2][i].lru_count = tracker_l2;
	        break;
	    }
    }
    uint32_t temp = (convert_tag_l2 << index_len_l2) | convert_index_l2;
    level_select = 2;
    if(param.PREF_N !=0)
        pref_hit_miss(param, temp, flag_hit);
    if(pref_miss==1 && param.PREF_N!=0)
        write_misses_l2++;
    level_select = 0;
    //Handling write misses
    if(flag_hit == 0){
        if(param.PREF_N==0)
	        write_misses_l2++;
	    for(i=0;i<param.L2_ASSOC;i++){
	        if(cache_2[convert_index_l2][i].valid == 0){
	            to_evict_l2 = i;
	            break;
	        }
	        else{
		        flag_evict++;
	        }
	    }
	    if(flag_evict == param.L2_ASSOC){
            level_select = 2;
	        to_evict_l2 = lru_update(cache_2, convert_index_l2, param);
            level_select = 0;
	    }
	    if(cache_2[convert_index_l2][to_evict_l2].valid == 1 && cache_2[convert_index_l2][to_evict_l2].dirty == 1){
	        writebacks_l2++;
	    }
	    cache_2[convert_index_l2][to_evict_l2].valid = 1;
	    cache_2[convert_index_l2][to_evict_l2].dirty = 1;
	    cache_2[convert_index_l2][to_evict_l2].set_nu = convert_index_l2;
	    cache_2[convert_index_l2][to_evict_l2].tag = convert_tag_l2;
	    cache_2[convert_index_l2][to_evict_l2].lru_count = tracker_l2;
    }
    pref_hit =0;
    pref_miss =0;
}  
void array_arrange(cache_blocks *cache_print, cache_params_t param){
    int i,j;

    cache_blocks a; 
    uint32_t loop_condition = 0;    

    if(level_select == 1){
        loop_condition = param.L1_ASSOC;
    }
    else if(level_select == 2){
	loop_condition = param.L2_ASSOC;
    }
    for(i=0;i<loop_condition;++i){
	for(j=i+1;j<loop_condition;++j){
	    if(cache_print[i].lru_count<cache_print[j].lru_count){
	        a = cache_print[i];
	        cache_print[i] = cache_print[j];
		cache_print[j] = a;
	    }
	}
    }
} 

void pref_sort(cache_params_t param){
    pref_params a;
    if(param.PREF_N>1){
        for(i=0; i<param.PREF_N;++i){
            for(j=i+1;j<param.PREF_N;++j){
                if(pref[i].lru_count<pref[j].lru_count){
                    a = pref[i];
                    pref[i] = pref[j];
                    pref[j] = a;
                }
            }
        }
    }
}

void stream_update(cache_params_t param, int col, int row, uint32_t addr){
    int i,j;
    if(pref_hit == 1){
        if(row !=0){
            for(i = 0; i<row+1;i++){
                prefetcher[i][col] = 0;
            }
        }
        int j =0;
        for(i = row+1; i<param.PREF_M; i++){
            prefetcher[j][col] = prefetcher[i][col];
            j++;
        }
        for(i =j; i < param.PREF_M; i++){
            if(i != 0){
                prefetcher[i][col] = addr + i + 1;
            }
            else{
                prefetcher[i][col] = addr+ i + 1;
            }
            if(level_select == 1){
                prefetches_l1++;
            }
            else if(level_select ==2)
                prefetches_l2++;
        }
    }
    else if(pref_miss == 1){
        for(i=0;i<param.PREF_N;i++){
            if(pref[i].index == col){
                pref[i].valid = 1;
            }
        }
        for(i =0; i<param.PREF_M;i++){
            if(level_select==2)
                prefetches_l2++;
            else if(level_select == 1)
                prefetches_l1++;
            prefetcher[i][col] = addr + i + 1;

        }
    }
}

void pref_hit_miss(cache_params_t param, uint32_t addr, int flag){
    int i,j;
    pref_lru_count++;
    pref_sort(param);
    //Hit
    for(i=0; i<param.PREF_N; i++){
        for(j=0;j<param.PREF_M; j++){
            if((prefetcher[j][pref[i].index] == addr) && (pref[i].valid == 1)){
                pref_hit = 1;
                stream_update(param,pref[i].index,j,addr);
                pref[i].lru_count = pref_lru_count;
                break;
            }            
        }
        if(pref_hit == 1){
            break;
        }
    }
    if(pref_hit == 0 && flag == 0){
        pref_miss = 1;
        int valid_flag=0;
        int col;
        for(i =0; i<param.PREF_N;i++){
            if(pref[i].valid == 1){
                valid_flag++;
            }
        }
        if(valid_flag == param.PREF_N){
            col = pref[param.PREF_N-1].index;
            pref[param.PREF_N-1].lru_count = pref_lru_count;
        }
        else{
            col = pref[valid_flag].index;
            pref[valid_flag].lru_count = pref_lru_count;
        }
        stream_update(param,col,0,addr);
        valid_flag = 0;
    }
}

