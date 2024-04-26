
#ifndef MEMORY_RECYCLER
#define MEMORY_RECYCLER

#include "types.h"

typedef struct memory_recycler_struct memory_recycler;

memory_recycler* memory_recycler_create(
  allocator_fn allocator, 
  destructor_fn destructor
);

void* memory_recycler_get(memory_recycler* mr);

void memory_recycler_reuse(memory_recycler* mr, void* item);

// Will not destroy any created memory that has not been returned to the 
// pool via a reuse call.
void memory_recycler_destroy(memory_recycler* mr);

#endif
