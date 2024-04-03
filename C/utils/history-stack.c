
#include <stdlib.h>
#include <stdint.h>

#include "../core/api-provider.h"

#include "history-stack.h"

struct history_stack_struct {
	void** stack;
	uint8_t i;	
	uint8_t size;
};

history_stack* history_stack_create(uint8_t size) {
  history_stack* s = malloc(sizeof(history_stack));
  if (s == NULL) {
    PD.api->system->error("Could not allocate struct for new history stack");
  }
  s->i = 0;
  s->size = size;
  s->stack = malloc(size * sizeof(void *));
  if (s->stack == NULL) {
    PD.api->system->error("Could not allocate child array in history stack");
  }
  for (uint8_t i=0; i < size; i++) {
    s->stack[i] = NULL;
  }
  return s;
}

void history_stack_push(history_stack* s, void* item) {
	s->stack[s->i] = item;
	s->i = s->i + 1 < s->size ? s->i + 1 : 0;
}

void* history_stack_pop(history_stack* s) {
	uint8_t prev = s->i > 0 ? s->i - 1 : s->size - 1;
	void* item = s->stack[prev];
	if (!item) {
		return NULL;
	}
	s->i = prev;
	s->stack[prev] = NULL;
	return item;
}

void history_stack_destroy(history_stack* s) {
	free(s->stack);
	free(s);
}
