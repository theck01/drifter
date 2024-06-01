
#ifndef INPUT_CONTROLS
#define INPUT_CONTROLS

#include "pd_api.h"

#include "C/utils/closure.h"

#include "event.h"

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

void controls_poll(controls* c);

void destroy_controls(controls* c);

#endif 
