
#ifndef CAMERA
#define CAMERA

#include "C/utils/geometry.h"

#include "controls.h"
#include "entity.h"
#include "world.h"

typedef enum {
  FIXED,
  MANUAL_MOVEMENT,
  TRACKING
} camera_mode_e;

typedef struct camera_struct camera;

camera* camera_create(world* w, point origin);

void camera_track(camera* c, entity* e);

void camera_control_with_dpad(camera* c, controls* dpad);

void camera_fix(camera* c);

camera_mode_e camera_get_mode(camera* c);

void camera_destroy(camera* c);

#endif 
