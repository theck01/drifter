
#include "C/api.h"
#include "C/core/utils/data-structures/history-stack.h"

#include "gesture.h"

struct gesture_struct {
  controls* parent;
  button_group_e btn_group;
  closure* recognizer;
  history_stack* input_history;
  gid_t listener_id;
};

void* gesture_handle_input(void* context, va_list args) {
  gesture* g = (gesture*)context;
  input_event* events = va_arg(args, input_event*);

  uintptr_t active_btns = 0;
  int i = 0;
  while (!input_event_is_nil(events[i])) {
    input_button_e btn = input_event_button(events[i]);
    input_action_e action = input_event_action(events[i]);
    if (action == HELD || action == PRESS || action == TAP) {
      active_btns |= btn;
    }
    i++;
  }
  history_stack_push(g->input_history, (void*)active_btns);
  closure_call(
    g->recognizer, 
    g->input_history
  );

  return NULL;
}

gesture* gesture_create(
    controls* c, 
    button_group_e btn_group,
    closure* recognizer, 
    uint8_t buffer_size
) {
  gesture* g = malloc(sizeof(gesture));
  if (!g) {
    get_api()->system->error("Could not allocate memory for gesture");
  }

  g->parent = c;
  g->btn_group = btn_group;
  g->recognizer = recognizer;
  g->input_history = history_stack_create(buffer_size);

  closure* btn_listener = closure_create(g, gesture_handle_input);
  g->listener_id = controls_add_realtime_listener_for_button_group(
    c,
    closure_create(g, gesture_handle_input),
    btn_group
  );

  return g;
}

void gesture_destroy(gesture* g) {
  controls_remove_realtime_listener_for_button_group(
    g->parent, 
    g->listener_id, 
    g->btn_group
  );
  history_stack_destroy(g->input_history);
  closure_destroy(g->recognizer);
  free(g);
}
