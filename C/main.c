//
//  main.c
//  Extension
//
//  Created by Dan Messing on 5/01/18.
//  Copyright (c) 2018 Panic, Inc. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pd_api.h"

#include "C/core/api-provider.h"
#include "C/utils/cyclic-array.h"
#include "C/utils/history-stack.h"

static PlaydateAPI* pd = NULL;

uint8_t count;

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

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(
  PlaydateAPI* playdate, 
  PDSystemEvent event, 
  uint32_t arg
) {
	(void)arg;

	if ( event == kEventInitLua )
	{
    PD.api = playdate;
    cyclic_array* test = cyclic_array_create(3, &create_test_item);
    history_stack* stack = history_stack_create(3);
    for (uint8_t i = 0; i < 7; i++) {
      test_item * item = cyclic_array_next(test);
      item->id = i;
      item->label = values[i];
      history_stack_push(stack, item);
    };
    for (uint8_t i = 0; i < 4; i++) {
      test_item* item = cyclic_array_next(test);
      if (item) {
        playdate->system->logToConsole("Test item: { id: %d, label: %s }\n", item->id, item->label);
      } else {
        playdate->system->logToConsole("NULL item");
      }
      test_item* stack_item = history_stack_pop(stack);
      if (stack_item) {
        playdate->system->logToConsole("Stack item: { id: %d, label: %s }\n", stack_item->id, stack_item->label);
      } else {
        playdate->system->logToConsole("NULL stack item");
      }
    }
    cyclic_array_destroy(test);
    history_stack_destroy(stack);
	}

	return 0;
}
