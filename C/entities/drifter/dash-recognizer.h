
#pragma once

#include "C/core/closure.h"
#include "C/core/controls/controls.h"


typedef struct dash_recognizer_struct dash_recognizer;

/*
 * Closures:
 * (direction_e dash_direction) called when a double tap on the dpad creates
 * a dash event in the given direction.
 */
dash_recognizer* dash_recognizer_create(
  controls* c, 
  closure* dash_listener
);

void dash_recognizer_destroy(dash_recognizer* g);
