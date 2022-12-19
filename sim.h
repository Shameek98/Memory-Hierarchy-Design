#ifndef SIM_CACHE_H
#define SIM_CACHE_H

typedef
struct {
   uint32_t BLOCKSIZE;
   uint32_t L1_SIZE;
   uint32_t L1_ASSOC;
   uint32_t L2_SIZE;
   uint32_t L2_ASSOC;
   uint32_t PREF_N;
   uint32_t PREF_M;
} cache_params_t;

typedef struct {
   uint32_t valid;
   uint32_t dirty;
   uint32_t tag;
   uint32_t set_nu;
   uint32_t lru_count;

} cache_blocks;

typedef struct{
   uint32_t valid;
   uint32_t index;
   uint32_t lru_count;
} pref_params;

//global func declarations
//--L1 stuff
void read_l1(cache_blocks **cache_1, cache_blocks **cache_2, cache_params_t param);
void write_l1(cache_blocks **cache_1, cache_blocks **cache_2, cache_params_t param);
//int lru_update_l1(cache_blocks **cache, cache_params_t param);

//--L2 stuff
void read_l2(cache_blocks **cache_2, cache_params_t param);
void write_l2(cache_blocks **cache_1, cache_blocks **cache_2, int to_evict, cache_params_t param);
int lru_update(cache_blocks **cache, uint32_t index, cache_params_t param);
void array_arrange(cache_blocks *cache_print, cache_params_t param);
void pref_hit_miss(cache_params_t param, uint32_t addr, int flag);
void stream_update(cache_params_t param, int col, int row, uint32_t addr);
void pref_sort(cache_params_t param);
// Put additional data structures here as per your requirement.

#endif