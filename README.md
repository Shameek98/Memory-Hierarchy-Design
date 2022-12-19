# Memory-Hierarchy-Design
Implemented a flexible cache and memory hierarchy simulator and used it to compare the performance, area, and energy of different memory hierarchy configurations. The cache uses the LRU replacement policy and write-back + write-allocate write policy. The cache is augmented with a prefetch unit using a stream buffer.

Designed a generic cache module that can be used at any level in a memory hierarchy. For example, this cache module can be “instantiated” as an L1 cache, an L2 cache, an L3 cache, and so on.

Configurable parameters: Size of the cache(SIZE), Associativity of the cache(ASSOC), Size of a cache block(BLOCKSIZE).
Replacement policy: LRU
Write policy: write-back+write-allocate

Have also augmented a cache using a configurable stream buffer prefetcher.
