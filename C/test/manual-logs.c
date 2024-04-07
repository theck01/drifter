#include "pd_api.h"

#include "C/core/api-provider.h"
#include "C/utils/memory-pool.h"
#include "C/utils/vector.h"

char *values[10] = {
  "Hello",
  "my",
  "baby",
  "hello",
  "my",
  "honey",
  "hello",
  "my",
  "ragtime",
  "gal"
};



typedef struct test_item_struct {
  uint8_t id;
  char * label;
} test_item;

void* create_test_item(void) {
  return malloc(sizeof(test_item));
}

uint8_t next_test_id = 2;
test_item stack_item_1 = {
  .label = "Chuck Jones",
  .id = 0
};
test_item stack_item_2 = {
  .label = "Michigan J. Frog",
  .id = 1
};

int8_t compare8(void *i) {
  test_item* ti = (test_item*)i;
  return ti->id < 8 ? 1 : (ti->id > 8 ? -1 : 0);
}

void run_tests(void) {
  PlaydateAPI* api = get_api();

  // Setup
  memory_pool* test = memory_pool_create(10, &create_test_item);
  vector* v = vector_create(3);
  for (uint8_t i = 0; i < 10; i++) {
    test_item* item = memory_pool_next(test);
    item->id = next_test_id++;
    item->label = values[i];
    vector_push(v, item);
  };
  vector_insert_at_index(v, &stack_item_1, 0);
  vector_insert_at_index(v, &stack_item_2, 1);

  // Vector
  bsearch_result r = vector_bsearch(v, &compare8);
  api->system->logToConsole(
    "bsearch result: { id: %d, label: %s } at index: %d", 
    ((test_item*)r.item)->id, 
    ((test_item*)r.item)->label, 
    r.index
  );
  vector_remove_at_index(v, r.index);
  bsearch_result empty = vector_bsearch(v, &compare8);
  api->system->logToConsole(
    "bsearch result: %sNULL item at index: %d", 
    empty.item ? "non-" : "",
    empty.index
  );
  uint16_t length = vector_length(v);
  for (uint8_t i = 0; i < length; i++) {
    test_item* item = vector_item_at_index(v, i);
    api->system->logToConsole("Vector item: { id: %d, label: %s }", item->id, item->label);
  }

  vector_destroy(v);
  memory_pool_destroy(test);
}
