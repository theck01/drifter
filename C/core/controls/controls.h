
#ifndef INPUT_CONTROLS
#define INPUT_CONTROLS

#include "pd_api.h"

#include "C/utils/closure.h"
#include "C/utils/types.h"

#include "input-event.h"

typedef enum {
  DPAD = 0,
  A_BTN = 1,
  B_BTN = 2,
} button_group_e;

typedef struct controls_struct controls;

controls* create_controls(void);

/*
 * Closures:
 * listener(input_event*): Called when a relevant button has an event, with
 *   a readonly list of events ending with a "nil" input event (see event.h)
 */
gid_t controls_add_listener_for_button_group(
  controls* c, 
  closure* listener,
  button_group_e group 
);

void controls_remove_listener_for_button_group(
  controls* c, 
  gid_t listener_id,
  button_group_e group 
);

// Same as the regular listener, but will be called on every frame the controls
// are active even if there are no events
gid_t controls_add_realtime_listener_for_button_group(
  controls* c, 
  closure* listener,
  button_group_e group 
);
void controls_remove_realtime_listener_for_button_group(
  controls* c, 
  gid_t listener_id,
  button_group_e group 
);

/*
 * Closures:
 * listener(crank_event* e): Called when cranking causes a change in tick count
 */
gid_t controls_add_crank_listener(controls* c, closure* listener);

void controls_remove_crank_listener(controls* c, gid_t listener_id);

// Notify controls of new button events in a nil-event terminated array, and of
// change in crank position if a significant enough deviation occured.
void controls_handle(
  controls* c, 
  input_event* button_events, 
  crank_event* cranked
);

void destroy_controls(controls* c);

#endif 
