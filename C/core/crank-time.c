
#include <stdbool.h>
#include <math.h>

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/utils/closure.h"

#include "event-emitter.h"

#include "crank-time.h"

static float degrees_per_tick = 0;
static int8_t last_tick = -1;
static event_emitter* emitter = NULL;

void initialize_if_needed(void) {
  if (!emitter) {
    degrees_per_tick = 360.0f / CRANK_TICKS_PER_REVOLUTION;

    float last_angle = get_api()->system->getCrankAngle();
    uint8_t raw_tick = roundf(last_angle / degrees_per_tick);
    last_tick = raw_tick == CRANK_TICKS_PER_REVOLUTION ? 0 : raw_tick;

    emitter = event_emitter_create();
  }
}

gid_t crank_time_add_listener(closure* c) {
  initialize_if_needed();
  return event_emitter_add(emitter, c);
}

void crank_time_remove_listener(gid_t id) {
  initialize_if_needed();
  event_emitter_remove(emitter, id);
}

void crank_time_update(void) {
  initialize_if_needed();

  float current_angle = get_api()->system->getCrankAngle();
  float last_tick_angle = degrees_per_tick * last_tick;

  // Check special case if crank passed 0* boundary, assumes user cannot
  // crank at 1/2 rotation per frame, and that there are plenty of ticks per
  // revolution (~5+).
  bool wrapped_zero_boundary = fabsf(current_angle - last_tick_angle) > 180;
  bool angle_increasing = wrapped_zero_boundary ?
    current_angle < 180 :
    current_angle > last_tick_angle;

  uint8_t raw_tick = angle_increasing ?
    floorf(current_angle / degrees_per_tick) :
    ceilf(current_angle/degrees_per_tick);
  uint8_t current_tick = raw_tick == CRANK_TICKS_PER_REVOLUTION ? 0 : raw_tick;

  if (current_tick != last_tick) {
    int8_t time_correction = angle_increasing ?
      (current_tick < last_tick ? CRANK_TICKS_PER_REVOLUTION : 0) :
      (current_tick > last_tick ? -CRANK_TICKS_PER_REVOLUTION : 0);
    int8_t time_diff = current_tick - last_tick + time_correction;

    event_emitter_fire(emitter, time_diff);
    last_tick = current_tick;
  }
}
