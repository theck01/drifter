
#ifndef INPUT_EVENTS
#define INPUT_EVENTS

#include <stdbool.h>
#include <stdint.h>

#include "pd_api.h"

typedef enum {
  PRESS = 1<<0,
  HELD = 1<<1,
  RELEASE = 1<<2,
  TAP = 1<<3,
} input_action_e;

typedef enum {
  D_UP = 1<<4,
  D_RIGHT = 1<<5,
  D_DOWN = 1<<6,
  D_LEFT = 1<<7,
  A = 1<<8,
  B = 1<<9
} input_button_e;

typedef uint16_t input_event;

input_event create_input_event(input_action_e action, input_button_e button);
input_event create_nil_event(void);

bool input_event_is_nil(input_event e);

input_action_e input_event_action(input_event e);
input_button_e input_event_button(input_event e);

bool input_button_is_dpad(input_button_e btn);

typedef struct crank_event_struct {
  int tick;
  int diff;
} crank_event;

#endif
