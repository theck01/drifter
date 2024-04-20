
#include <stdlib.h>

#include "C/api.h"
#include "C/macro.h"

#include "vector.h"

uint16_t MAX_CAPACITY = 16384; // 2^14 elements, plenty for a PlayDate

struct vector_struct {
  void** array;
  uint16_t length;
  uint16_t capacity;
};

vector* vector_create(uint16_t initial_capacity) {
  vector* v = malloc(sizeof(vector));
  if (v == NULL) {
    get_api()->system->error("Could not allocate struct for new vector");
  }

  initial_capacity = min(initial_capacity, MAX_CAPACITY);
  uint16_t two_power_cap = 4;
  while(two_power_cap < initial_capacity) two_power_cap *= 2;
  void** array = malloc(two_power_cap * sizeof(void *));
  if (array == NULL) {
    get_api()->system->error("Could not allocate array for new vector");
  }
  for (uint16_t i=0; i < two_power_cap; i++) array[i] = NULL;
  v->array = array;
  v->capacity = two_power_cap;
  v->length = 0;

  return v;
}

uint16_t vector_length(vector* v) {
  return v->length;
}

void* vector_item_at_index(vector* v, uint16_t i) {
  return i < v->length ? v->array[i] : NULL;
}

void vector_push(vector* v, void* item) {
  vector_insert_at_index(v, item, v->length);
}


void vector_maybe_resize_for_new_item(vector* v) {
  uint16_t length = v->length;
  uint16_t cap = v->capacity;
  if (length < cap) return;

  if (cap == MAX_CAPACITY) {
    get_api()->system->error("Vector reached maximum capacity");
  }

  uint16_t new_cap = cap * 2;
  void** new_array = malloc(new_cap * sizeof(void *));
  if (new_array == NULL) {
    get_api()->system->error("Could not allocate array for vector resize");
  }

  void** old_array = v->array;
  for (uint16_t i=0; i<new_cap; i++) {
    new_array[i] = i < length ? old_array[i] : NULL;
  }

  v->array = new_array;
  v->capacity = new_cap;

  free(old_array);
  return;
}

void vector_insert_at_index(vector* v, void* item, uint16_t i) {
  uint16_t length = v->length;

  vector_maybe_resize_for_new_item(v);

  // Clamp insertion index to vector length, there should be no
  // holes in the vector.
  i = min(i, length);

  // Shift entries after insertion point by 1
  void** array = v->array;
  for (
    uint16_t shift_i = length - 1; 
    shift_i >= i && shift_i < UINT16_MAX; 
    shift_i--
  ) {
    array[shift_i+1] = array[shift_i]; 
  }

  array[i] = item;
  v->length = length + 1;
}

void* vector_pop(vector* v) {
  return vector_remove_at_index(v, v->length - 1);
}

void* vector_remove_at_index(vector* v, uint16_t i) {
  uint16_t length = v->length;
  void** array = v->array;
  if (i >= length) {
    return NULL;
  }

  void* item = array[i];

  // Shift entries after removal point by -1
  for (
    uint16_t shift_i = i;
    shift_i < length; 
    shift_i++
  ) {
    array[shift_i] = shift_i < length - 1 ? array[shift_i+1] : NULL;
  }

  v->length = length - 1;
  return item;
}

bsearch_result vector_bsearch(vector* v, compare_fn compare, void* userdata) {
  void** array = v->array;
  int32_t start = 0;
  int32_t end = v->length-1;
  while(start <= end) {
    int32_t i = ((end - start) / 2) + start;
    int8_t comparison = compare(array[i], userdata);
    if (comparison < 0) {
      end = i-1;
    } else if (comparison > 0) {
      start = i+1;
    } else {
      return (bsearch_result) {
        .item = array[i],
        .index = i
      };
    }
  }

  return (bsearch_result) { 
    .item = NULL,
    .index = UINT16_MAX 
  };
}

void vector_destroy(vector* v) {
  free(v->array);
  free(v);
}
