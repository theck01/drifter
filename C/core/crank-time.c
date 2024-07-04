
#include <stdbool.h>
#include <math.h>

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/utils/closure.h"
#include "C/utils/history-stack.h"

#include "event-emitter.h"
#include "sprite-animator.h"

#include "crank-time.h"

const float TICKS_PER_RATE_INCREMENT = 2;
const int ROLLING_FRAME_WINDOW = 12;

// for absolute tick calculation

static float degrees_per_tick = 0;
static int8_t last_tick = -1;
static event_emitter* emitter = NULL;
static int current_time = 0;

// rate state
typedef enum {
	ZERO = 0,
	MIN = 1,
	LOW = 2,
	NORMAL_A = 3,
	NORMAL_B = 4,
	HI = 5,
	MAX = 6
} tick_rate_e;
static int raw_rate = 0;
static tick_rate_e current_rate = 0;
static uint8_t rolling_frame_count = 0;

void initialize_if_needed(void) {
  if (!emitter) {
    degrees_per_tick = 360.0f / CRANK_TICKS_PER_REVOLUTION;

    float last_angle = get_api()->system->getCrankAngle();
    uint8_t raw_tick = roundf(last_angle / degrees_per_tick);
    last_tick = raw_tick == CRANK_TICKS_PER_REVOLUTION ? 0 : raw_tick;

    emitter = event_emitter_create();
  }
}

gid_t crank_time_advance_listener(closure* c) {
  initialize_if_needed();
  return event_emitter_add(emitter, c);
}

void crank_time_remove_listener(gid_t id) {
  initialize_if_needed();
  event_emitter_remove(emitter, id);
}

void crank_time_update(void) {
  initialize_if_needed();
	rolling_frame_count = (rolling_frame_count+1) % ROLLING_FRAME_WINDOW;

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
  int8_t tick_diff = current_tick - last_tick + tick_correction;
  last_tick = current_tick;

	int ticks = 0;
	raw_rate = max(
		min(raw_rate + tick_diff, MAX * TICKS_PER_RATE_INCREMENT), 
		ZERO
	);
	current_rate = raw_rate / TICKS_PER_RATE_INCREMENT;
	int is1in3 = (rolling_frame_count % 3 == 0) ? 1 : 0; 
	int is1in2 = (rolling_frame_count % 2 == 0) ? 1 : 0;
	int is1in4 = (rolling_frame_count % 4 == 0) ? 1 : 0; 
	int is2in3 = (is1in2 || is1in3);
	int is3in4 = (is2in3 || rolling_frame_count % 5 == 0) ? 1 : 0;
	int is5in6 = (is3in4 || rolling_frame_count % 7 == 0) ? 1 : 0;
	int is7in12 = (is1in2 || rolling_frame_count % 5 == 0) ? 1 : 0;
	switch(current_rate) {
		case MIN:
			ticks = is1in4;
			break;
		case LOW:
			ticks = is7in12;
			break;
		case NORMAL_A:
		case NORMAL_B:
			ticks++;
			break;
		case HI:
			ticks = 1 + is5in6;
			break;
		case MAX:
			ticks = 2 + is1in2;
			break;
		case ZERO:
		default:
			break;
	}

  // The first event will always be a START
  crank_mask_e mask = START;
  while (ticks > 0) {
    // If this is the last diff to apply, include END in the mask (maybe
    // combining with START if there is only 1 tick to process)
    if (ticks == 1) {
      mask |= END;
    }
    event_emitter_fire(emitter, ++current_time, mask);
    ticks--;
    mask = 0;
  }
}
