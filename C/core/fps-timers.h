
#ifndef FPS_TIMER
#define FPS_TIMER

#include <stdint.h>
#include <stdbool.h>

typedef void (*callback_fn)(void*);
uint32_t fps_timer_start(callback_fn callback, void* customdata, uint8_t fps, bool loop);

void fps_timer_stop(uint32_t timerId, uint8_t fps);

void fps_timers_update(void);

#endif
