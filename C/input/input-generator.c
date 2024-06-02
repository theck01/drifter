#include <stdbool.h>
#include <stdint.h>

#include "pd_api.h"

#include "C/api.h"
#include "C/const.h"

#include "event.h"

#include "input-generator.h"

PDButtons buttons_on_last_flush = 0;
typedef struct raw_event_struct {
  PDButtons btn;
  bool is_pressed;
} raw_event;
static raw_event input_queue[8 /* INPUT_QUEUE_SIZE */];
static uint8_t queue_size = 0; 

typedef struct button_conversion_struct {
  PDButtons pd;
  input_button_e local;
} button_pair;

static button_pair ALL_BUTTON_LIST[] = {
  { .pd = kButtonLeft, .local = D_LEFT },
  { .pd = kButtonUp, .local = D_UP },
  { .pd = kButtonRight, .local = D_RIGHT },
  { .pd = kButtonDown, .local = D_DOWN },
  { .pd = kButtonA, .local = A },
  { .pd = kButtonB, .local = B }
};

int button_callback(
  PDButtons button, 
  int is_pressed, 
  uint32_t when, 
  void* userdata
) {
  if (queue_size >= INPUT_QUEUE_SIZE) {
    get_api()->system->error("Input queue overflow");
  }

  input_queue[queue_size].btn = button;
  input_queue[queue_size].is_pressed = is_pressed;
  queue_size++;
  return 0;
}


void input_generator_listen(void) {
  get_api()->system->setButtonCallback(button_callback, NULL, INPUT_QUEUE_SIZE);
}

bool events_for_button_pair(
  button_pair* p, 
  input_event* results, 
  PDButtons end_button_state
) {
  int event_count = 0;
  for (int i = 0; i < queue_size; i++) {
    if (input_queue[i].btn == p->pd) {
      if (event_count > 0) {
        input_action_e past_action = input_event_action(
          results[event_count - 1]
        );
        if (input_queue[i].is_pressed) {
          // If the button was repressed within the same frame, drop the RELEASE
          // event for the button so it is just considered as held.
          if (past_action == RELEASE) {
            results[--event_count] = create_nil_event();
          } else {
            // Otherwise add a PRESS event
            results[event_count++] = create_input_event(PRESS, p->local);
          }
        } else {
          // If the button was released within the same frame, convert the event
          // to a TAP
          if (past_action == PRESS) {
            results[event_count - 1] = create_input_event(TAP, p->local);
          } else {
            // Otherwise add a PRESS event
            results[event_count++] = create_input_event(RELEASE, p->local);
          }
        }
      } else {
        if (input_queue[i].is_pressed) {
          results[event_count++] = create_input_event(PRESS, p->local);
        } else {
          results[event_count++] = create_input_event(RELEASE, p->local);
        }
      }
    }
  }

  if (event_count == 0 && (end_button_state & p->pd)) {
    results[0] = create_input_event(HELD, p->local);
    event_count = 1;
  }

  results[event_count] = create_nil_event();
  return event_count > 0;
}

void input_generator_flush(controls* c) {
  PDButtons next_state = buttons_on_last_flush;
  for (int i = 0; i < queue_size; i++) {
    next_state = input_queue[i].is_pressed
      ? (next_state | input_queue[i].btn)
      : (next_state & (~input_queue[i].btn));
  }

  int event_count = 0;
  // +BUTTON_COUNT for HELD buttons that have no new raw events
  input_event all_events[INPUT_QUEUE_SIZE + BUTTON_COUNT];
  input_event button_events[INPUT_QUEUE_SIZE];

  for (int i = 0; i < BUTTON_COUNT; i++) {
    if (
      events_for_button_pair(&ALL_BUTTON_LIST[i], button_events, next_state)
    ) {
      int j = 0;
      while (!input_event_is_nil(button_events[j])) {
        all_events[event_count++] = button_events[j++];
        if (event_count > INPUT_QUEUE_SIZE + BUTTON_COUNT) {
          get_api()->system->error("Input generator event overflow");
        }
      }
    }
  }
  all_events[event_count] = create_nil_event();
  if (event_count > 0) {
    controls_handle(c, all_events);
  }

  queue_size = 0;
  buttons_on_last_flush = next_state;
}
