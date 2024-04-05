#include "pd_api.h"

#include "C/core/api-provider.h"
#include "C/utils/memory-pool.h"
#include "C/utils/history-stack.h"
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

void run_tests(void) {
  PlaydateAPI* api = get_api();

  // Setup
  memory_pool* test = memory_pool_create(10, &create_test_item);
  history_stack* stack = history_stack_create(3);
  vector* v = vector_create(3);
  for (uint8_t i = 0; i < 10; i++) {
    test_item* item = memory_pool_next(test);
    item->id = next_test_id++;
    item->label = values[i];
    history_stack_push(stack, item);
    vector_push(v, item);
  };

  // Memory pool
  for (uint8_t i = 0; i < 12; i++) {
    test_item* item = memory_pool_next(test);
    if (item) {
      api->system->logToConsole("Test item: { id: %d, label: %s }\n", item->id, item->label);
    } else {
      api->system->logToConsole("NULL item");
    }
  }

  // Vector
  vector_insert_at_index(v, &stack_item_1, 0);
  vector_insert_at_index(v, &stack_item_2, 1);
  test_item* second_my = vector_remove_at_index(v, 6);
  test_item* first_my = vector_remove_at_index(v, 3);
  api->system->logToConsole("First \"my\" item: { id: %d, label: %s }\n", first_my->id, first_my->label);
  api->system->logToConsole("Second \"my\" item: { id: %d, label: %s }\n", second_my->id, second_my->label);
  uint16_t length = vector_length(v);
  api->system->logToConsole("Vector length: %d", length);
  for (uint8_t i = 0; i < length; i++) {
    test_item* item = vector_item_at_index(v, i);
    api->system->logToConsole("Vector item: { id: %d, label: %s }\n", item->id, item->label);
  }

  // History stack
  for (uint8_t i = 0; i < 4; i++) {
    test_item* stack_item = history_stack_pop(stack);
    if (stack_item) {
      api->system->logToConsole("Stack item: { id: %d, label: %s }\n", stack_item->id, stack_item->label);
    } else {
      api->system->logToConsole("NULL stack item");
    }
  }
  history_stack_destroy(stack);
  vector_destroy(v);
  memory_pool_destroy(test);
}
