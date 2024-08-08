
#include "C/api.h"
#include "C/const.h"
#include "C/core/utils/geometry.h"
#include "C/core/viewport/viewport.h"

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

void entity_sprite_update_z_index(LCDSprite* sprite) {
  PlaydateAPI* api = get_api();
  if (!api->sprite->isVisible(sprite)) {
    return;
  }

  float _, y;
  api->sprite->getPosition(sprite, &_, &y);
  point offset;
  viewport_get_offset(&offset);

  api->sprite->setZIndex(
    sprite,
    ENTITY_Z_INDEX + ((int)y - offset.y)
  );
}

LCDSprite* create_entity_sprite(void) {
  PlaydateAPI* api = get_api();
  LCDSprite* s = api->sprite->newSprite();
  api->sprite->setUpdateFunction(s, entity_sprite_update_z_index);
  api->sprite->setUpdatesEnabled(s, 1);
  api->sprite->setCollisionsEnabled(s, 0);
  get_api()->sprite->setCenter(s, 0.5 /* middle x */, 1 /* bottom y */);
  return s;
}
