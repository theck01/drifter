
#include "C/const.h"
#include "C/macro.h"
#include "C/utils/vector.h"

#include "api-provider.h"

#include "fps-timers.h"

typedef struct timer_struct {
  uint8_t fps;
  uint16_t last_observed_frame;
  vector* callbacks;
} timer;

typedef struct callback_struct {
  fps_timer_id id;
  callback_fn fn;
  void* customdata;
  bool loop;
} callback;

static timer timer_by_fps[51];
static bool initialized = false;
static fps_timer_id next_callback_id = 0;

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

uint16_t time_to_frames(float time, uint8_t fps) {
  return (uint16_t)(time * fps);
}

fps_timer_id fps_timer_start(callback_fn fn, void* customdata, uint8_t fps, bool loop) {
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
    return UINT16_MAX;
  }
  c->id = next_callback_id++;
  c->fn = fn;
  c->customdata = customdata;
  c->loop = loop;
  vector_push(t->callbacks, c);
  return c->id;
}


int8_t compare_callback_ids(void* item, void* search_id) {
  callback* c = (callback *)item;
  fps_timer_id id = *((fps_timer_id*) search_id);
  return c->id < id ? 1 : (c->id > id ? -1 : 0);
}

void fps_timer_replace(
  fps_timer_id timer_id, 
  uint8_t fps, 
  callback_fn fn, 
  void* customdata
) {
  fps_timer_initialize_if_needed();
  if (timer_id == INVALID_TIMER_ID) {
    get_api()->system->error("Attempting to replace an invalid timer ID");
    return;
  }

  timer t = timer_by_fps[fps];
  bsearch_result r = vector_bsearch(
    t.callbacks, 
    &compare_callback_ids, 
    &timer_id
  );
  if (!r.item) {
    get_api()->system->error(
      "Could not find timer to replace for id: %d, fps: %d", 
      timer_id,
      fps
    );
  }

  callback* c = vector_item_at_index(t.callbacks, r.index);
  c->fn = fn;
  c->customdata = customdata;
}

void fps_timer_stop(fps_timer_id timer_id, uint8_t fps) {
  fps_timer_initialize_if_needed();
  if (timer_id == INVALID_TIMER_ID) {
    get_api()->system->error("Attempting to stop an invalid timer ID");
    return;
  }

  timer t = timer_by_fps[fps];
  bsearch_result r = vector_bsearch(
    t.callbacks, 
    &compare_callback_ids, 
    &timer_id
  );
  if (!r.item) {
    get_api()->system->error(
      "Could not find timer to stop for id: %d, fps: %d", 
      timer_id,
      fps
    );
  }

  callback* c = vector_remove_at_index(t.callbacks, r.index);
  free(c);
}

void fps_timers_update(void) {
  float now = get_api()->system->getElapsedTime();
  for (uint8_t fps=1; fps<=MAX_FPS; fps++) {
    timer* t = &timer_by_fps[fps];
    if (!t->callbacks) {
      continue;
    }

    uint16_t now_frame = time_to_frames(now, fps);
    if (t->last_observed_frame == now_frame) {
      continue;
    }

    uint16_t length = vector_length(t->callbacks);
    for (uint16_t i=0; i < length; i++) {
      callback* c = vector_item_at_index(t->callbacks, i);
      if (!c->fn) {
        continue;
      }
      c->fn(c->customdata);
      if (!c->loop) {
        fps_timer_stop(c->id, fps);
      }
    }

    t->last_observed_frame = now_frame; 
  }
}
