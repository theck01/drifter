
#include <stdbool.h>
#include <math.h>

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/utils/closure.h"
#include "C/utils/history-stack.h"

#include "crank.h"
#include "event-emitter.h"
#include "sprite-animator.h"

#include "game-clock.h"

const float TICKS_PER_RATE_INCREMENT = 2;
const int ROLLING_FRAME_WINDOW = 12;

static event_emitter* emitter = NULL;
static gid_t crank_listener_id = INVALID_GID;
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

void* game_clock_cranked(void* _, va_list args) {
  int ticks = va_arg(args, int);
	raw_rate = max(
		min(raw_rate + ticks, MAX * TICKS_PER_RATE_INCREMENT), 
		ZERO
	);
	current_rate = raw_rate / TICKS_PER_RATE_INCREMENT;
  return NULL;
}

static void initialize_if_needed(void) {
  if (!emitter) {
    crank_listener_id = crank_add_listener(
        closure_create(NULL, game_clock_cranked)
    );
    emitter = event_emitter_create();
  }
}

gid_t game_clock_add_listener(closure* c) {
  initialize_if_needed();
  return event_emitter_add(emitter, c);
}

void game_clock_remove_listener(gid_t id) {
  initialize_if_needed();
  event_emitter_remove(emitter, id);
}

void game_clock_update(void) {
  initialize_if_needed();
	rolling_frame_count = (rolling_frame_count+1) % ROLLING_FRAME_WINDOW;

	int is1in3 = (rolling_frame_count % 3 == 0) ? 1 : 0; 
	int is1in2 = (rolling_frame_count % 2 == 0) ? 1 : 0;
	int is1in4 = (rolling_frame_count % 4 == 0) ? 1 : 0; 
	int is2in3 = (is1in2 || is1in3);
	int is3in4 = (is2in3 || rolling_frame_count % 5 == 0) ? 1 : 0;
	int is5in6 = (is3in4 || rolling_frame_count % 7 == 0) ? 1 : 0;
	int is7in12 = (is1in2 || rolling_frame_count % 5 == 0) ? 1 : 0;

	int ticks = 0;
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
  clock_mask_e mask = START;
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
