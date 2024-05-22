
#include <stdbool.h>
#include <math.h>

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/utils/closure.h"
#include "C/utils/history-stack.h"

#include "event-emitter.h"

#include "crank-time.h"

static const uint8_t HISTORY_SIZE_TPS = 4;
static const float HISTORY_DURATION_SEC = HISTORY_SIZE_TPS / 30.0f;
static const float MINIMUM_MULTIPLIER_TPS = 35.0f;
static const float MAX_CONSISTENCY_MULTIPLIER = 3.0f;
static const float MAX_SPEED_MULTIPLIER = 1.6f;
// VALUEx over crank speed required to increment speed multiplier 1x
static const float OVERSPEED_RATIO = 2.2f; 
static const float CONSISTENCY_MULTIPLIER_INCREMENT_PER_TICK = 
  1.0f / (2 * CRANK_TICKS_PER_REVOLUTION);

// for absolute tick calculation

static float degrees_per_tick = 0;
static int8_t last_tick = -1;
static event_emitter* emitter = NULL;

// for crank multiplier relative tick

static history_stack* tick_history;
static float average_tps = 0;
static int16_t running_diff_sum = 0;
static float consistency_multiplier = 1.0f;

void initialize_if_needed(void) {
  if (!emitter || !tick_history) {
    degrees_per_tick = 360.0f / CRANK_TICKS_PER_REVOLUTION;

    float last_angle = get_api()->system->getCrankAngle();
    uint8_t raw_tick = roundf(last_angle / degrees_per_tick);
    last_tick = raw_tick == CRANK_TICKS_PER_REVOLUTION ? 0 : raw_tick;

    emitter = event_emitter_create();
    tick_history = history_stack_create(HISTORY_SIZE_TPS);
    for (uint8_t i=0; i<HISTORY_SIZE_TPS; i++) {
      history_stack_push(tick_history, (void *)0);
    }
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

  int8_t tick_correction = 0;
  if (angle_increasing && (current_tick < last_tick)) {
    tick_correction = CRANK_TICKS_PER_REVOLUTION;
  } else if (!angle_increasing && (current_tick > last_tick)) {
    tick_correction = -CRANK_TICKS_PER_REVOLUTION;
  }
  intptr_t tick_diff = current_tick - last_tick + tick_correction;
  last_tick = current_tick;

  intptr_t oldest_diff = (intptr_t)history_stack_push(tick_history, (void*)tick_diff);
  running_diff_sum += tick_diff - oldest_diff;
  average_tps = fabsf(running_diff_sum / HISTORY_DURATION_SEC);

  float speed_multiplier = 1.0f;
  if (average_tps > MINIMUM_MULTIPLIER_TPS) {
    consistency_multiplier = min( 
      (
        consistency_multiplier + 
        (tick_diff > 0 ? tick_diff : -tick_diff) * 
        CONSISTENCY_MULTIPLIER_INCREMENT_PER_TICK
      ),
      MAX_CONSISTENCY_MULTIPLIER
    );
    speed_multiplier = min(
      1 + (average_tps - MINIMUM_MULTIPLIER_TPS) / (OVERSPEED_RATIO * MINIMUM_MULTIPLIER_TPS),
      MAX_SPEED_MULTIPLIER
    );
  } else {
    // decay consistency multiplier by a ratio based on how close the crank
    // rate is to the multiplier
    consistency_multiplier = max(
      consistency_multiplier * average_tps / MINIMUM_MULTIPLIER_TPS,
      1.0f
    );
  }

  int8_t applied_diff = min(
    max(
      roundf(tick_diff * consistency_multiplier * speed_multiplier),
      INT8_MIN
    ),
    INT8_MAX
  );
  if (applied_diff != 0) {
    event_emitter_fire(emitter, applied_diff);
  }
}
