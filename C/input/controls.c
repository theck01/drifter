#include "C/api.h"
#include "C/core/event-emitter.h"

#include "controls.h"

struct controls_struct {
  event_emitter* dpad;
  event_emitter* a;
  event_emitter* b;
};

typedef struct button_conversion_struct {
  PDButtons pd;
  input_button_e local;
} button_pair;

static button_pair DPAD_BUTTON_LIST[4] = {
  { .pd = kButtonLeft, .local = D_LEFT },
  { .pd = kButtonUp, .local = D_UP },
  { .pd = kButtonRight, .local = D_RIGHT },
  { .pd = kButtonDown, .local = D_DOWN }
};

controls* create_controls(void) {
  controls* c = malloc(sizeof(controls));
  if (!c) {
    get_api()->system->error("Could not allocate memory for controls");
  }

  c->dpad = event_emitter_create();
  c->a = event_emitter_create();
  c->b = event_emitter_create();

  return c;
}

gid_t controls_add_listener_for_button_group(
  controls* c, 
  closure* listener,
  button_group_e group 
) {
  switch(group) {
    case DPAD:
      return event_emitter_add(c->dpad, listener);
    case A_BTN:
      return event_emitter_add(c->a, listener);
    case B_BTN:
      return event_emitter_add(c->b, listener);
  }
  return INVALID_GID;
}

void controls_remove_listener_for_button_group(
  controls* c, 
  gid_t listener_id,
  button_group_e group 
) {
  switch(group) {
    case DPAD:
      event_emitter_remove(c->dpad, listener_id);
      break;
    case A_BTN:
      event_emitter_remove(c->a, listener_id);
      break;
    case B_BTN:
      event_emitter_remove(c->b, listener_id);
      break;
  }
}

bool action_for_button(
  PDButtons b, 
  PDButtons current, 
  PDButtons pushed, 
  PDButtons released,
  input_action_e* result
) {
  if (b & pushed) {
    *result = PRESS;
    return true;
  }

  if (b & current) {
    *result = HELD;
    return true;
  }

  if (b & released) {
    *result = RELEASE;
    return true;
  }

  return false;
}

void controls_poll(controls* c) {
  PlaydateAPI* api = get_api();

  PDButtons current, pushed, released;
  get_api()->system->getButtonState(&current, &pushed, &released);

  input_event event_list[4];
  input_action_e action;
  int event_count = 0;
  for (int i = 0; i<4; i++) {
    PDButtons pd_dpad =  DPAD_BUTTON_LIST[i].pd;
    input_button_e dpad =  DPAD_BUTTON_LIST[i].local;
    if (action_for_button(pd_dpad, current, pushed, released, &action)) {
      event_list[event_count++] = create_input_event(action, dpad);
    }
  }
  event_list[event_count] = create_nil_event();
  if (event_count > 0) {
    event_emitter_fire(c->dpad, event_list);
  }

  event_list[1] = create_nil_event();
  if (action_for_button(kButtonA, current, pushed, released, &action)) {
    event_list[0] = create_input_event(action, A);
    event_emitter_fire(c->a, event_list);
  }

  if (action_for_button(kButtonB, current, pushed, released, &action)) {
    event_list[0] = create_input_event(action, B);
    event_emitter_fire(c->b, event_list);
  }
}

void destroy_controls(controls* c) {
  event_emitter_destroy(c->dpad);
  event_emitter_destroy(c->a);
  event_emitter_destroy(c->b);
  free(c);
}
