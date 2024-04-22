
#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/utils/closure.h"
#include "C/utils/vector.h"

#include "event-emitter.h"

#include "fps-timers.h"

typedef struct timer_struct {
  uint8_t fps;
  uint16_t last_observed_frame;
  event_emitter* emitter;
} timer;

static timer timer_by_fps[51];
static bool initialized = false;

void fps_timer_initialize_if_needed(void) {
  if (initialized) {
    return;
  }

  for (uint8_t i=0; i <= MAX_FPS; i++) {
    timer_by_fps[i].fps = i;
    // Leave emitter at NULL so that the last_observed_frame of each
    // timer can be initialized correctly.
    timer_by_fps[i].emitter = NULL;
  }

  initialized = true;
}

static uint16_t time_to_frames(float time, uint8_t fps) {
  return (uint16_t)(time * fps);
}

static uint8_t clamp_fps(uint8_t fps) {
  return min(fps, MAX_FPS);
}

gid_t fps_timer_start(uint8_t fps, closure* c) {
  fps_timer_initialize_if_needed();

  fps = clamp_fps(fps);

  timer* t = &timer_by_fps[fps];
  if (!t->emitter) {
    t->emitter = event_emitter_create();

    float now = get_api()->system->getElapsedTime();
    t->last_observed_frame = time_to_frames(now, fps);
  }

  return event_emitter_add(t->emitter, c);
}

void fps_timer_stop(uint8_t fps, gid_t timer_id) {
  fps_timer_initialize_if_needed();
  fps = clamp_fps(fps);
  timer t = timer_by_fps[fps];
  event_emitter_remove(t.emitter, timer_id);
}

void fps_timers_update(void) {
  float now = get_api()->system->getElapsedTime();
  for (uint8_t fps=1; fps<=MAX_FPS; fps++) {
    timer* t = &timer_by_fps[fps];
    if (!t->emitter) {
      continue;
    }

    uint16_t now_frame = time_to_frames(now, fps);
    if (t->last_observed_frame == now_frame) {
      continue;
    }

    event_emitter_fire(t->emitter, NULL /* eventdata */);
    t->last_observed_frame = now_frame; 
  }
}
