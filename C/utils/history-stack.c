
#include <stdlib.h>
#include <stdint.h>

#include "C/api.h"

#include "history-stack.h"

struct history_stack_struct {
	void** stack;
	uint16_t i;	
	uint16_t size;
};

history_stack* history_stack_create(uint16_t size) {
  history_stack* s = malloc(sizeof(history_stack));
  if (s == NULL) {
    get_api()->system->error("Could not allocate struct for new history stack");
  }
  s->i = 0;
  s->size = size;
  s->stack = malloc(size * sizeof(void *));
  if (s->stack == NULL) {
    get_api()->system->error("Could not allocate child array in history stack");
  }
  for (uint16_t i=0; i < size; i++) {
    s->stack[i] = NULL;
  }
  return s;
}

void* history_stack_push(history_stack* s, void* item) {
  void* existing = s->stack[s->i];
	s->stack[s->i] = item;
	s->i = s->i + 1 < s->size ? s->i + 1 : 0;
  return existing;
}

void* history_stack_pop(history_stack* s) {
	uint16_t prev = s->i > 0 ? s->i - 1 : s->size - 1;
	void* item = s->stack[prev];
	s->i = prev;
	s->stack[prev] = NULL;
	return item;
}

uint16_t history_stack_size(history_stack* s) {
  return s->size;
}

void* history_stack_get(history_stack* s, uint16_t i) {
  if (i >= s->size) {
    get_api()->system->error(
      "Cannot get history stack item %d beyond %d size", 
      i,
      s->size
    );
  }
  uint16_t offset_i = ((uint32_t)s->i + (uint32_t)i)%(s->size);
  return s->stack[offset_i];
}

void history_stack_flush(history_stack* s) {
  for (uint16_t i = 0; i < s->size; i++) {
    s->stack[i] = NULL;
  }
}

void history_stack_destroy(history_stack* s) {
	free(s->stack);
	free(s);
}
