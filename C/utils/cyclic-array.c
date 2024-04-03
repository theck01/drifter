
#include <stdlib.h>
#include <stdint.h>

#include "../core/api-provider.h"

#include "cyclic-array.h"

struct cyclic_array_struct {
  void** a;
  uint8_t i;
  uint8_t size;
};

cyclic_array* cyclic_array_create(uint8_t size, element_allocator allocator) {
  cyclic_array* new_array = malloc(sizeof(cyclic_array));
  if (new_array == NULL && PD.api) {
    PD.api->system->error("Could not allocate struct for new cyclic array");
  }
  new_array->i = 0;
  new_array->size = size;
  new_array->a = malloc(size * sizeof(void *));
  if (new_array->a == NULL) {
    PD.api->system->error("Could not allocate child array in cyclic array");
  }
  for (uint8_t i=0; i < size; i++) {
    new_array->a[i] = (*allocator)();
  }
  return new_array;
}

void* cyclic_array_next(cyclic_array* array) {
  void * element = array->a[array->i];
  array->i = array->i + 1 < array->size ? array->i + 1 : 0;
  return element;
}

void cyclic_array_destroy(cyclic_array* array) {
  for (uint8_t i = 0; i < array->size; i++) {
    free(array->a[i]);
  }
  free(array->a);
  free(array);
}

