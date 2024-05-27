
#ifndef FPS_TIMER
#define FPS_TIMER

#include <stdint.h>

#include "C/utils/closure.h"
#include "C/utils/types.h"

/*
 * Closures:
 * listener(void): Called when timer fires at a rate determined by fps
 */
gid_t fps_timer_start(uint8_t fps, closure* c);

void fps_timer_stop(uint8_t fps, gid_t timerId);

void fps_timers_update(void);

#endif
