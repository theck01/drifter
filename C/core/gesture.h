
#ifndef GESTURE
#define GESTURE

#include <stdint.h>

#include "C/utils/closure.h"

#include "controls.h"

typedef struct gesture_struct gesture;

/*
 * Closures:
 * (uint16_t* active_btn_history)
 * Scan over the active buttons on each frame (with length buffer_size) to
 * recognize and handle complex input gestures
 */
gesture* gesture_create(
  controls* c, 
  button_group_e btn_group,
  closure* recognizer, 
  uint8_t buffer_size
);

void gesture_destroy(gesture* g);

#endif
