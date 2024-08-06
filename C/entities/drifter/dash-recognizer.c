
#include <stdbool.h>

#include "C/api.h"
#include "C/core/controls/gesture.h"
#include "C/core/controls/input-event.h"
#include "C/core/utils/types.h"
#include "C/core/utils/data-structures/history-stack.h"

#include "dash-recognizer.h"

static const uint8_t DASH_BUFFER_SIZE = 16; // ~ 2/3 second

static const uint8_t FRAME_DELTA_BETWEEN_DASHES = 3;

static const uint8_t COYOTE_FRAMES = 2;

// 3 changes in any buttons state within the buffer represent a full button
// press and an additional beginning of a second, resulting in a hopefully
// fluid dash when moving or standing still before or after the dash.
static const uint8_t DASH_DELTA_THRESHOLD = 3; 

typedef struct dash_coyote_struct {
  direction_e base;
  int8_t frames_remaining;
} dash_coyote;

struct dash_recognizer_struct {
  gesture* g;
  closure* listener;
  uint8_t delay_remaining;
  dash_coyote dc;
};

void* watch_double_tap(void* context, va_list args) {
  dash_recognizer* dr = (dash_recognizer*) context;
  history_stack* dpad_history = va_arg(args, history_stack*);

  if (dr->delay_remaining > 0) {
    dr->delay_remaining--;
    if (dr->delay_remaining == 0) {
      history_stack_flush(dpad_history);
      dr->dc.base = NONE;
      dr->dc.frames_remaining = -1;
    }
    return NULL;
  }

  PlaydateAPI* api = get_api();
  int d_up = 0, d_left = 0, d_right = 0, d_down = 0;
  uint8_t buffer_size = history_stack_size(dpad_history);
  for (int i = 0; i < (buffer_size-1); i++) {
    uintptr_t current = (uintptr_t)history_stack_get(dpad_history, i);
    uintptr_t next = (uintptr_t)history_stack_get(dpad_history, i+1);
    d_up += (current & D_UP) == (next & D_UP) ? 0 : 1;
    d_left += (current & D_LEFT) == (next & D_LEFT) ? 0 : 1;
    d_right += (current & D_RIGHT) == (next & D_RIGHT) ? 0 : 1;
    d_down += (current & D_DOWN) == (next & D_DOWN) ? 0 : 1;
  }

  uintptr_t current_btns = (uintptr_t)history_stack_get(
    dpad_history, 
    buffer_size - 1
  );
  bool includes_up = d_up || (current_btns & D_UP);
  bool includes_left = d_left || (current_btns & D_LEFT);
  bool includes_right = d_right || (current_btns & D_RIGHT);
  bool includes_down = d_down || (current_btns & D_DOWN);

  direction_e dir = dr->dc.base;

  if (d_up >= DASH_DELTA_THRESHOLD) {
    if (includes_left == includes_right) {
      dir = U;
    } else {
      dir = includes_left ? UL : UR;
    }
  } else if (d_left >= DASH_DELTA_THRESHOLD) {
    if (includes_up == includes_down) {
      dir = L;
    } else {
      dir = includes_up ? UL : DL;
    }
  } else if (d_right >= DASH_DELTA_THRESHOLD) {
    if (includes_up == includes_down) {
      dir = R;
    } else {
      dir = includes_up ? UR : DR;
    }
  } else if (d_down >= DASH_DELTA_THRESHOLD) {
    if (includes_left == includes_right) {
      dir = D;
    } else {
      dir = includes_left ? DL : DR;
    }
  }

  if (dr->dc.frames_remaining >= 0) { 
    if (dr->dc.frames_remaining == 0) { 
      closure_call(dr->listener, dir);
      dr->delay_remaining = FRAME_DELTA_BETWEEN_DASHES;
    }
    dr->dc.frames_remaining--;
  } else if (dir != NONE) {
    dr->dc.frames_remaining = COYOTE_FRAMES;
    dr->dc.base = dir;
  }

  return NULL;
}



/*
 * Closures:
 * (direction_e* dash_direction) called when a double tap on the dpad creates
 * a dash event in the given direction.
 */
dash_recognizer* dash_recognizer_create(
  controls* c, 
  closure* dash_listener
) {
  dash_recognizer* dr = malloc(sizeof(dash_recognizer));
  if (!dr) {
    get_api()->system->error("Could not allocate memory for dash_recognizer");
  }
  dr->g = gesture_create(
    c,
    DPAD,
    closure_create(dr, watch_double_tap),
    DASH_BUFFER_SIZE
  );
  dr->listener = dash_listener;
  return dr;
}

void dash_recognizer_destroy(dash_recognizer* dr) {
  gesture_destroy(dr->g);
  closure_destroy(dr->listener);
  free(dr);
}
