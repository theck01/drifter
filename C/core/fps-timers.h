
#ifndef FPS_TIMER
#define FPS_TIMER

#include <stdint.h>
#include <stdbool.h>

typedef uint16_t fps_timer_id;
static const fps_timer_id INVALID_TIMER_ID = UINT16_MAX;

typedef void (*callback_fn)(void*);
fps_timer_id fps_timer_start(
  callback_fn fn, 
  void* customdata, 
  uint8_t fps, 
  bool loop
);

void fps_timer_stop(fps_timer_id timerId, uint8_t fps);

// Cannot change fps, but can update callback function and data.
// More memory efficient than stop and start, although this gain is minor for
// no-loop timers and so drop the loop param assuming it will mostly be true.
void fps_timer_replace(
  fps_timer_id timerId, 
  uint8_t fps, 
  callback_fn fn, 
  void* customdata
);

void fps_timers_update(void);

#endif
