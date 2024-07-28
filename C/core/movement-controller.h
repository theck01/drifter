
#ifndef MOVEMENT_CONTROLLER
#define MOVEMENT_CONTROLLER

#include <stdint.h>

#include "C/utils/closure.h"

#include "controls.h"

typedef struct movement_config_struct {
  uint8_t max_speed_px;
  uint8_t speed_increment_px;
  uint8_t speed_decrement_px;
} movement_config;

typedef struct movement_controller_struct movement_controller;

// move(int dx, int dy): Will be called on every step with the offset that the
// movable object should be shifted by. If an offset of (0,0) is provided then
// active user input has ended
movement_controller* movement_controller_create(
  movement_config* config,
  closure* move, 
  controls* dpad
);

int movement_controller_get_speed(movement_controller* mc);

direction_e movement_controller_get_direction(movement_controller* mc);

// Advance movement by one time unit, should be called on all game clock
// ticks when the moveable object could shift
void movement_controller_step(movement_controller* mc);

void movement_controller_destroy(movement_controller* m);

#endif
