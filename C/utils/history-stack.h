
#ifndef HISTORY_STACK
#define HISTORY_STACK

#include <stdint.h>

typedef struct history_stack_struct history_stack;

history_stack* history_stack_create(uint16_t size);

void* history_stack_push(history_stack* stack, void* item);

void* history_stack_pop(history_stack* stack);

uint16_t history_stack_size(history_stack* stack);

void* history_stack_get(history_stack* stack, uint16_t i);

void history_stack_flush(history_stack* stack);

void history_stack_destroy(history_stack* stack);

#endif
