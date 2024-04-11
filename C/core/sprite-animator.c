
#include "api-provider.h"
#include "fps-timers.h"

#include "sprite-animator.h"

struct sprite_animator_struct {
  LCDSprite* sprite;
  LCDBitmapTable* animation;
  uint8_t fps;
  uint8_t frame;
  fps_timer_id timer_id;
  bool timer_active;
};

uint8_t sprite_animator_show_frame(sprite_animator* s, uint8_t frame) {
  PlaydateAPI* api = get_api();
  LCDBitmap* image = api->graphics->getTableBitmap(s->animation, frame);
  if (!image) {
    frame = 0;
    image = api->graphics->getTableBitmap(s->animation, 0);
  }
  int w,h;
  api->graphics->getBitmapData(image, &w, &h, NULL, NULL, NULL);
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
  s->sprite = sprite;
  s->animation = animation;
  s->fps=fps;
  s->timer_id = INVALID_TIMER_ID;
  s->timer_active = false;
  s->frame = sprite_animator_show_frame(s, starting_frame);
  return s;
}

void sprite_animator_tick(void* vs) {
  sprite_animator* s = (sprite_animator*)vs;
  s->frame = sprite_animator_show_frame(s, s->frame + 1);
}

void sprite_animator_start(sprite_animator* s) {
  if (s->timer_active) {
    get_api()->system->error("Sprite animator is already running");
    return;
  }
  if (s->timer_id == INVALID_TIMER_ID) {
    s->timer_id = fps_timer_start(sprite_animator_tick, s, s->fps, true);
  } else {
    fps_timer_replace(s->timer_id, s->fps, sprite_animator_tick, s);
  }
}

void sprite_animator_stop(sprite_animator* s) {
  if (!s->timer_active) {
    get_api()->system->error("Sprite animator is already stopped");
    return;
  }
  fps_timer_replace(s->timer_id, s->fps, NULL /* fn */, s);
}

void sprite_animator_set_animation(
  sprite_animator* s, 
  LCDBitmapTable* animation, 
  uint8_t starting_frame
) {
  s->animation = animation;
  s->frame = sprite_animator_show_frame(s, starting_frame);
}

void sprite_animator_destroy(sprite_animator* s) {
  fps_timer_stop(s->timer_id, s->fps);
  get_api()->sprite->freeSprite(s->sprite);
  free(s);
}
