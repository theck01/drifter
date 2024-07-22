#include "C/api.h"
#include "C/const.h"

#include "event-emitter.h"

#include "controls.h"

struct controls_struct {
  event_emitter* dpad;
  event_emitter* a;
  event_emitter* b;
  event_emitter* crank;
};

controls* create_controls(void) {
  controls* c = malloc(sizeof(controls));
  if (!c) {
    get_api()->system->error("Could not allocate memory for controls");
  }

  c->dpad = event_emitter_create();
  c->a = event_emitter_create();
  c->b = event_emitter_create();
  c->crank = event_emitter_create();

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

gid_t controls_add_crank_listener(controls* c, closure* listener) {
  return event_emitter_add(c->crank, listener);
}

void controls_remove_crank_listener(controls* c, gid_t listener_id) {
  event_emitter_remove(c->crank, listener_id);
}


void controls_handle(controls* c, input_event* btn_events, crank_event* ce) {
  if (btn_events) {
    input_event dpad_events[INPUT_QUEUE_SIZE];
    int dpad_count = 0;
    input_event a_events[INPUT_QUEUE_SIZE];
    int a_count = 0;
    input_event b_events[INPUT_QUEUE_SIZE];
    int b_count = 0;

    int i = 0;
    while (!input_event_is_nil(btn_events[i])) {
      input_button_e btn = input_event_button(btn_events[i]);
      if (input_button_is_dpad(btn)) {
        dpad_events[dpad_count++] = btn_events[i];
      } else if (btn == A) {
        a_events[a_count++] = btn_events[i];
      } else if (btn == B) {
        b_events[b_count++] = btn_events[i];
      }
      i++;
    }
    dpad_events[dpad_count] = create_nil_event();
    if (dpad_count > 0) {
      event_emitter_fire(c->dpad, dpad_events);
    }
    a_events[a_count] = create_nil_event();
    if (a_count > 0) {
      event_emitter_fire(c->a, a_events);
    }
    b_events[b_count] = create_nil_event();
    if (b_count > 0) {
      event_emitter_fire(c->b, b_events);
    }
  }

  if (ce) {
    event_emitter_fire(c->crank, ce);
  }
}

void destroy_controls(controls* c) {
  event_emitter_destroy(c->dpad);
  event_emitter_destroy(c->a);
  event_emitter_destroy(c->b);
  free(c);
}
