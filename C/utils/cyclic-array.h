
#ifndef CYCLIC_ARRAY
#define CYCLIC_ARRAY

typedef struct cyclic_array_struct cyclic_array;

typedef void* (*element_allocator)(void);
cyclic_array* cyclic_array_create(uint8_t size, element_allocator allocator);

void* cyclic_array_next(cyclic_array *array);

void cyclic_array_destroy(cyclic_array* array);

#endif
