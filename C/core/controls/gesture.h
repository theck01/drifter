
#ifndef GESTURE
#define GESTURE

#include <stdint.h>

#include "C/core/closure.h"

#include "controls.h"

typedef struct gesture_struct gesture;

/*
 * Closures:
 * (history_stack* active_btn_history)
 * active_btn_history items can be cast to a uintptr_t, which represents a bit
 * field of all buttons actively pressed on the given frame. This closure should
 * scan over the active buttons on each frame to recognize and handle complex
 * input gestures.
 */
gesture* gesture_create(
  controls* c, 
  button_group_e btn_group,
  closure* recognizer, 
  uint8_t buffer_size
);

void gesture_destroy(gesture* g);

#endif
