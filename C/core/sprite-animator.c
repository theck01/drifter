#include <stdbool.h>

#include "C/api.h"
#include "C/macro.h"
#include "C/utils/closure.h"

#include "fps-timers.h"

#include "sprite-animator.h"

static int global_pause_counter = 0;

struct sprite_animator_struct {
  LCDSprite* sprite;
  LCDBitmapTable* animation;
  uint8_t fps;
  uint8_t frame;
  gid_t timer_id;
  bool paused;
};

uint8_t sprite_animator_show_frame(sprite_animator* s, uint8_t frame) {
  PlaydateAPI* api = get_api();
  LCDBitmap* image = api->graphics->getTableBitmap(s->animation, frame);
  if (!image) {
    frame = 0;
    image = api->graphics->getTableBitmap(s->animation, 0);
  }
  api->sprite->setImage(s->sprite, image, kBitmapUnflipped);
  return frame;
}

sprite_animator* sprite_animator_create(
  LCDSprite* sprite,
  LCDBitmapTable* animation, 
  uint8_t fps,
  uint8_t starting_frame
) {
  sprite_animator* s = malloc(sizeof(sprite_animator));
  if (!s) {
    get_api()->system->error("Could not allocate memory for sprite animator");
  }
  s->sprite = sprite;
  s->animation = animation;
  s->fps=fps;
  s->timer_id = INVALID_GID;
  s->frame = sprite_animator_show_frame(s, starting_frame);
  s->paused = false;
  return s;
}

void* sprite_animator_tick(void* animator, va_list _) {
  sprite_animator* s = (sprite_animator*)animator;
  if (global_pause_counter || s->paused) {
    return NULL;
  }
  s->frame = sprite_animator_show_frame(s, s->frame + 1);
  return NULL;
}

void sprite_animator_start(sprite_animator* s) {
  if (s->timer_id != INVALID_GID) {
    get_api()->system->error("Sprite animator is already running");
    return;
  }
  s->timer_id = fps_timer_start(
    s->fps, 
    closure_create(s, sprite_animator_tick)
  );
}

void sprite_animator_stop(sprite_animator* s) {
  if (s->timer_id == INVALID_GID) {
    get_api()->system->error("Sprite animator is already stopped");
    return;
  }
  fps_timer_stop(s->fps, s->timer_id);
  s->timer_id = INVALID_GID;
}

void sprite_animator_set_animation(
  sprite_animator* s, 
  LCDBitmapTable* animation
) {
  sprite_animator_set_animation_and_frame(s, animation, s->frame);
}

void sprite_animator_set_animation_and_frame(
  sprite_animator* s, 
  LCDBitmapTable* animation, 
  uint8_t starting_frame
) {
  s->animation = animation;
  s->frame = sprite_animator_show_frame(s, starting_frame);
}

void sprite_animator_destroy(sprite_animator* s) {
  sprite_animator_stop(s);
  free(s);
}

void sprite_animator_pause(sprite_animator* s) {
  s->paused = true;
}

void sprite_animator_resume(sprite_animator* s) {
  s->paused = false;
}

void sprite_animator_global_pause(void) {
  global_pause_counter++;
}

void sprite_animator_global_resume(void) {
  global_pause_counter = max(global_pause_counter - 1, 0);
}
