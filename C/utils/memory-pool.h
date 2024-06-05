
#ifndef MEMORY_POOL
#define MEMORY_POOL

#include "closure.h"
#include "types.h"

typedef struct memory_pool_struct memory_pool;

/*
 * Closures:
 *   allocator(void): create the item in the pool
 *   destructor(void* item): destroy the item in the pool
 */
memory_pool* memory_pool_create(
  uint16_t size, 
  closure* allocator,
  closure* destructor
);

void* memory_pool_next(memory_pool* array);

void memory_pool_destroy(memory_pool* array);

#endif
