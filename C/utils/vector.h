
#ifndef VECTOR
#define VECTOR

#include <stdbool.h>
#include <stdint.h>

typedef struct vector_struct vector;

vector* vector_create(uint16_t initial_capacity);

uint16_t vector_length(vector* v);

void* vector_item_at_index(vector* v, uint16_t i);

void vector_push(vector* v, void* item);
void* vector_pop(vector* v);
// Inserts the item at index i, shifting items at and after +1 position.
void vector_insert_at_index(vector* v, void* item, uint16_t i);
// Removes the item at index i, shifting items after -1 position.
void* vector_remove_at_index(vector* v, uint16_t i);


typedef bool (*find_fn)(void *);
typedef struct find_result_struct {
  void* item;
  uint16_t index;
} find_result;
find_result vector_find_index(vector* v, find_fn find);

void vector_destroy(vector* v);

#endif
