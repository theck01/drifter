
#ifndef DPAD_MOVEMENT
#define DPAD_MOVEMENT

#include <stdint.h>

#include "C/utils/closure.h"

#include "controls.h"

typedef struct movement_config_struct {
  uint8_t max_speed_px;
  uint8_t speed_increment_px;
  uint8_t speed_decrement_px;
} movement_config;

typedef struct dpad_movement_struct dpad_movement;

// move(int dx, int dy): Will be called on every step with the offset that the
// movable object should be shifted by. If an offset of (0,0) is provided then
// active user input has ended
dpad_movement* dpad_movement_create(
  movement_config* config,
  closure* move, 
  controls* dpad
);

int dpad_movement_get_speed(dpad_movement* dm);

direction_e dpad_movement_get_direction(dpad_movement* dm);

// Advance movement by one time unit, should be called on all game clock
// ticks when the moveable object could shift
void dpad_movement_step(dpad_movement* dm);

void dpad_movement_destroy(dpad_movement* dm);

#endif
