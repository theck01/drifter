
#ifndef MEMORY_POOL
#define MEMORY_POOL

typedef struct memory_pool_struct memory_pool;

typedef void* (*element_allocator)(void);
memory_pool* memory_pool_create(uint8_t size, element_allocator allocator);

void* memory_pool_next(memory_pool* array);

void memory_pool_destroy(memory_pool* array);

#endif
