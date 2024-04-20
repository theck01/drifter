
#ifndef FPS_TIMER
#define FPS_TIMER

#include <stdint.h>
#include <stdbool.h>

#include "C/utils/types.h"

typedef void (*callback_fn)(void*);
gid_t fps_timer_start(
  callback_fn fn, 
  void* customdata, 
  uint8_t fps, 
  bool loop
);

void fps_timer_stop(gid_t timerId, uint8_t fps);

// Cannot change fps, but can update callback function and data.
// More memory efficient than stop and start, although this gain is minor for
// no-loop timers and so drop the loop param assuming it will mostly be true.
void fps_timer_replace(
  gid_t timerId, 
  uint8_t fps, 
  callback_fn fn, 
  void* customdata
);

void fps_timers_update(void);

#endif
