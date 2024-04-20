
#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/utils/vector.h"

#include "fps-timers.h"

typedef struct timer_struct {
  uint8_t fps;
  uint16_t last_observed_frame;
  vector* callbacks;
} timer;

typedef struct callback_struct {
  gid_t id;
  callback_fn fn;
  void* customdata;
  bool loop;
  bool dead;
} callback;

static timer timer_by_fps[51];
static bool initialized = false;

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

gid_t fps_timer_start(callback_fn fn, void* customdata, uint8_t fps, bool loop) {
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
    return INVALID_GID;
  }
  c->id = getNextGID();
  c->fn = fn;
  c->customdata = customdata;
  c->loop = loop;
  c->dead = false;
  vector_push(t->callbacks, c);
  return c->id;
}


static int8_t compare_callback_ids(void* item, void* search_id) {
  callback* c = (callback *)item;
  gid_t id = *((gid_t*) search_id);
  return c->id < id ? 1 : (c->id > id ? -1 : 0);
}

void fps_timer_replace(
  gid_t timer_id, 
  uint8_t fps, 
  callback_fn fn, 
  void* customdata
) {
  fps_timer_initialize_if_needed();
  if (timer_id == INVALID_GID) {
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

void fps_timer_stop(gid_t timer_id, uint8_t fps) {
  fps_timer_initialize_if_needed();
  if (timer_id == INVALID_GID) {
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

  // Do not immediately remove the callback, so that the update method
  // can iterate through all callbacks linearly without having to keep track
  // of shifts if callbacks create or destroy one another.
  callback* c = vector_item_at_index(t.callbacks, r.index);
  c->dead = true;
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

    // Recheck callback length on each loop iteration, callbacks can create 
    // eachother (but only mark dead not destroy, so i never has to reverse)
    for (uint16_t i=0; i < vector_length(t->callbacks); i++) {
      callback* c = vector_item_at_index(t->callbacks, i);
      if (!c->fn || c->dead) {
        continue;
      }
      c->fn(c->customdata);
      c->dead = !c->loop;
    }

    t->last_observed_frame = now_frame; 
  }
}
