
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

typedef bool (*filter_fn)(void* vector_item, void* userdata);
typedef void (*cleanup_fn)(void* vector_item);
void vector_filter(
  vector* v, 
  filter_fn filter, 
  cleanup_fn clean, 
  void* userdata
);

// Should return 1 to search later in the vector, -1 to search earlier, and 0
// for direct matches.
typedef int8_t (*compare_fn)(void* vector_item, void* userdata);
typedef struct bsearch_result_struct {
  void* item;
  uint16_t index;
} bsearch_result;
bsearch_result vector_bsearch(vector* v, compare_fn compare, void* userdata);

void vector_destroy(vector* v);

#endif
