
#include <stdlib.h>
#include <stdint.h>

#include "C/api.h"

#include "memory-pool.h"

struct memory_pool_struct {
  void** a;
  uint8_t i;
  uint8_t size;
};

memory_pool* memory_pool_create(uint8_t size, element_allocator allocator) {
  memory_pool* new_array = malloc(sizeof(memory_pool));
  if (new_array == NULL) {
    get_api()->system->error("Could not allocate struct for new cyclic array");
  }
  new_array->i = 0;
  new_array->size = size;
  new_array->a = malloc(size * sizeof(void *));
  if (new_array->a == NULL) {
    get_api()->system->error("Could not allocate child array in cyclic array");
  }
  for (uint8_t i=0; i < size; i++) {
    new_array->a[i] = (*allocator)();
  }
  return new_array;
}

void* memory_pool_next(memory_pool* array) {
  void * element = array->a[array->i];
  array->i = array->i + 1 < array->size ? array->i + 1 : 0;
  return element;
}

void memory_pool_destroy(memory_pool* array) {
  for (uint8_t i = 0; i < array->size; i++) {
    free(array->a[i]);
  }
  free(array->a);
  free(array);
}

