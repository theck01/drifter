
#include "C/api.h"
#include "C/core/utils/data-structures/vector.h"

#include "memory-recycler.h"

struct memory_recycler_struct {
  vector* unused_pool;
  allocator_fn allocator;
  destructor_fn destructor;
};

memory_recycler* memory_recycler_create(
  allocator_fn allocator, 
  destructor_fn destructor
) {
  memory_recycler* mr = malloc(sizeof(memory_recycler));
  if (!mr) {
    get_api()->system->error(
      "Could not allocate memory for memory_recycler"
    );
  }
  mr->unused_pool = vector_create(1);
  mr->allocator = allocator;
  mr->destructor = destructor;
  return mr;

}

void* memory_recycler_get(memory_recycler* mr) {
  void* item = vector_pop(mr->unused_pool);
  if (!item) {
    item = mr->allocator();
  }
  if (!item) {
    get_api()->system->error(
      "Could not allocate new memory for memory_recycler"
    );
  }
  return item;
}

void memory_recycler_reuse(memory_recycler* mr, void* item) {
  vector_push(mr->unused_pool, item);
}

void memory_recycler_destroy(memory_recycler* mr) {
  uint16_t length = vector_length(mr->unused_pool);
  for (uint16_t i = 0; i < length; i++) {
    mr->destructor(vector_item_at_index(mr->unused_pool, i));
  }
  vector_destroy(mr->unused_pool);
  free(mr);
}
