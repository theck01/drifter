
#include "C/api.h"

#include "sprite.h"

static void noop_sprite_update(LCDSprite* s) {}

LCDSprite* create_draw_only_sprite(void) {
  PlaydateAPI* api = get_api();
  LCDSprite* s = api->sprite->newSprite();
  api->sprite->setUpdateFunction(s, noop_sprite_update);
  api->sprite->setUpdatesEnabled(s, 0);
  api->sprite->setCollisionsEnabled(s, 0);
  return s;
}
