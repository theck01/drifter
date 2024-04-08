
#include "C/const.h"
#include "C/macro.h"
#include "C/utils/vector.h"

#include "api-provider.h"

#include "fps-timers.h"

typedef struct timer_struct {
  uint8_t fps;
  uint32_t last_observed_frame;
  vector* callbacks;
} timer;

typedef struct callback_struct {
  uint32_t id;
  callback_fn fn;
  void* customdata;
  bool loop;
} callback;

static timer timer_by_fps[51];
static bool initialized = false;
static uint32_t next_callback_id = 0;

void fps_timer_initialize_if_needed(void) {
  if (initialized) {
    return;
  }

  for (uint8_t i=0; i <= MAX_FPS; i++) {
    timer_by_fps[i].fps = i;
    timer_by_fps[i].callbacks = NULL;
  }

  initialized = true;
}

uint32_t time_to_frames(float time, uint8_t fps) {
  return (uint32_t)(time * fps);
}

uint32_t fps_timer_start(callback_fn fn, void* customdata, uint8_t fps, bool loop) {
  fps_timer_initialize_if_needed();

  fps = min(fps, MAX_FPS);

  timer* t = &timer_by_fps[fps];
  if (!t->callbacks) {
    t->callbacks = vector_create(1);
    float now = get_api()->system->getElapsedTime();
    t->last_observed_frame = time_to_frames(now, fps);
  }
  callback* c = malloc(sizeof(callback));
  if (!c) {
    get_api()->system->error("Could not allocate memory for fps timer callback");
    return UINT32_MAX;
  }
  c->id = next_callback_id++;
  c->fn = fn;
  c->customdata = customdata;
  c->loop = loop;
  vector_push(t->callbacks, c);
  return c->id;
}


static uint32_t searchId = 0;
int8_t compareCallbackIds(void* i) {
  callback* c = (callback *)i;
  return c->id < searchId ? 1 : (c->id > searchId ? -1 : 0);
}

void fps_timer_stop(uint32_t timerId, uint8_t fps) {
  fps_timer_initialize_if_needed();

  searchId = timerId;
  timer* t = &timer_by_fps[fps];
  bsearch_result r = vector_bsearch(t->callbacks, &compareCallbackIds);
  if (!r.item) {
    get_api()->system->error(
      "Could not find timer for id: %d, fps: %d", 
      timerId,
      fps
    );
  }

  callback* c = vector_remove_at_index(t->callbacks, r.index);
  free(c);
}

void fps_timers_update(void) {
  float now = get_api()->system->getElapsedTime();
  for (uint8_t fps=1; fps<=MAX_FPS; fps++) {
    timer* t = &timer_by_fps[fps];
    if (!t->callbacks) {
      continue;
    }

    uint32_t now_frame = time_to_frames(now, fps);
    if (t->last_observed_frame == now_frame) {
      continue;
    }

    uint16_t length = vector_length(t->callbacks);
    for (uint16_t i=0; i < length; i++) {
      callback* c = vector_item_at_index(t->callbacks, i);
      c->fn(c->customdata);
      if (!c->loop) {
        fps_timer_stop(c->id, fps);
      }
    }

    t->last_observed_frame = now_frame; 
  }
}
