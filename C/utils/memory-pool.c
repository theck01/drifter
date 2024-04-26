
#include <stdlib.h>
#include <stdint.h>

#include "C/api.h"

#include "memory-pool.h"

struct memory_pool_struct {
  void** a;
  uint16_t i;
  uint16_t size;
  destructor_fn destructor;
};

memory_pool* memory_pool_create(
  uint16_t size, 
  allocator_fn allocator,
  destructor_fn destructor
) {
  memory_pool* pool = malloc(sizeof(memory_pool));
  if (pool == NULL) {
    get_api()->system->error("Could not allocate struct for new memory pool");
  }
  pool->i = 0;
  pool->size = size;
  pool->destructor = destructor;
  pool->a = malloc(size * sizeof(void *));
  if (pool->a == NULL) {
    get_api()->system->error("Could not allocate child array in cyclic array");
  }
  for (uint16_t i=0; i < size; i++) {
    pool->a[i] = (*allocator)();
  }
  return pool;
}

void* memory_pool_next(memory_pool* pool) {
  void * element = pool->a[pool->i];
  pool->i = pool->i + 1 < pool->size ? pool->i + 1 : 0;
  return element;
}

void memory_pool_destroy(memory_pool* pool) {
  for (uint16_t i = 0; i < pool->size; i++) {
    pool->destructor(pool->a[i]);
  }
  free(pool->a);
  free(pool);
}

