#include <math.h>

#include "pd_api.h"

#include "C/api.h"
#include "C/macro.h"
#include "C/core/event-emitter.h"

#include "viewport.h"

static point offset = { .x = 0, .y = 0 };
static point hold_bonus = { .x = 0, .y = 0 };
static const float HOLD_FRACTION = 0.25f;
static const int MAX_BONUS = 4 / HOLD_FRACTION;
static event_emitter* emitter;

static void maybe_init(void) {
  if (!emitter) {
    emitter = event_emitter_create();
  }
}

void viewport_get_offset(point* p) {
  p->x = offset.x;
  p->y = offset.y;
}

void viewport_set_offset(int x, int y) {
  maybe_init();

  if (offset.x == x && offset.y == y) {
    return;
  }

  offset.x = x;
  offset.y = y;
  /*
   * Playdate graphics originate from the top left of the screen, with x
   * and y increasing down and right
   * Playdate screen offsets are the opposite, increasing offsets shifts
   * the screen up and left.
   * Invert the applied offset sent to Playdate so viewport offset matches
   * matches the pixel positioning.
   */
  get_api()->graphics->setDrawOffset(-x, -y);
  event_emitter_fire(emitter, x, y);
}

gid_t viewport_add_offset_listener(closure* listener) {
  maybe_init();
  return event_emitter_add(emitter, listener);
}

void viewport_remove_offset_listener(gid_t listener_id) {
  maybe_init();
  event_emitter_remove(emitter, listener_id);
}

void dpad_handler(void* context, va_list args) {
  input_event* events = va_arg(args, input_event*);
  for (int i = 0; !input_event_is_nil(events[i]); i++) {
    point base_offset = { .x = 0, .y = 0 };
    input_button_e button = input_event_button(events[i]);
    switch (button) {
      case D_UP:
        base_offset.y = -1;
        break;
      case D_DOWN:
        base_offset.y = 1;
        break;
      case D_LEFT:
        base_offset.x = -1;
        break;
      case D_RIGHT:
        base_offset.x = 1;
        break;
    }

    point multiplier = { .x = 1, .y = 1 };
    input_action_e action = input_event_action(events[i]);
    switch (action) {
      case HELD:
        hold_bonus.x = max(
          min(hold_bonus.x + base_offset.x, MAX_BONUS), 
          -MAX_BONUS
        );
        hold_bonus.y = max(
          min(hold_bonus.y + base_offset.y, MAX_BONUS), 
          -MAX_BONUS
        );
        multiplier.x = powf(2, (hold_bonus.x > 0 ? hold_bonus.x : -hold_bonus.x) * HOLD_FRACTION);
        multiplier.y = powf(2, (hold_bonus.y > 0 ? hold_bonus.y : -hold_bonus.y) * HOLD_FRACTION);
        // purposefull fallthrough
      case PRESS:
      case TAP:
        viewport_set_offset(
          offset.x + (base_offset.x * multiplier.x), 
          offset.y + (base_offset.y * multiplier.y)
        );
        break;
    }

    if (action == RELEASE) {
      switch (button) {
        case D_UP:
          hold_bonus.y = max(hold_bonus.y, 0);
          break;
        case D_DOWN:
          hold_bonus.y = min(hold_bonus.y, 0);
          break;
        case D_LEFT:
          hold_bonus.x = max(hold_bonus.x, 0);
          break;
        case D_RIGHT:
          hold_bonus.x = min(hold_bonus.x, 0);
          break;
        }
    }
  }
}

void viewport_connect(controls* c) {
  // Implement disconnect as needed, but the actively polled controls object
  // should change depending on context, rather than listeners updating.
  controls_add_listener_for_button_group(
    c,
    closure_create(NULL /* context */, dpad_handler),
    DPAD
  );
}
