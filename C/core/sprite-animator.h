
#ifndef SPRITE_ANIMATOR
#define SPRITE_ANIMATOR

#include <stdbool.h>
#include <stdint.h>

#include "pd_api.h"

typedef struct sprite_animator_struct sprite_animator;

sprite_animator* sprite_animator_create(
  LCDSprite* s,
  LCDBitmapTable* animation, 
  uint8_t fps,
  uint8_t starting_frame
);

void sprite_animator_start(sprite_animator* s);


void sprite_animator_stop(sprite_animator* s);

void sprite_animator_set_animation(
  sprite_animator* s, 
  LCDBitmapTable* animation
);
void sprite_animator_set_animation_and_frame(
  sprite_animator* s, 
  LCDBitmapTable* animation, 
  uint8_t starting_frame
);

void sprite_animator_destroy(sprite_animator* s);

void sprite_animator_pause(void);
void sprite_animator_resume(void);

#endif
