
#ifndef MEMORY_POOL
#define MEMORY_POOL

#include "types.h"

typedef struct memory_pool_struct memory_pool;

memory_pool* memory_pool_create(
  uint16_t size, 
  allocator_fn allocator,
  destructor_fn destructor
);

void* memory_pool_next(memory_pool* array);

void memory_pool_destroy(memory_pool* array);

#endif
