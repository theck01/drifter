
#include "C/api.h"
#include "C/utils/functions.h"

#include "entity.private.h"

#include "tile.private.h"

LCDBitmap* white_bg = NULL;
LCDBitmap* black_bg = NULL;

static void init_if_needed() {
  PlaydateAPI* api = get_api();
  if (white_bg && black_bg) {
    return;
  }

  const char* err;
  white_bg = api->graphics->loadBitmap(
    "img/white-tile-bg.png",
    &err
  );
  if (!white_bg) {
    api->system->error(
      "Could not load white tile background image, err: %s", 
      err
    );
  }

  black_bg = api->graphics->loadBitmap(
    "img/black-tile-bg.png",
    &err
  );
  if (!black_bg) {
    api->system->error(
      "Could not load black tile background image, err: %s", 
      err
    );
  }
}

void init_tile(tile* t, int row, int col) {
  init_if_needed();
  PlaydateAPI* api = get_api();

  t->background = api->sprite->newSprite();
  api->sprite->setCenter(t->background, 0, 0);
  api->sprite->setZIndex(t->background, TILE_BACKGROUND_Z_INDEX);
  api->sprite->setImage(t->background, black_bg, kBitmapUnflipped);
  api->sprite->setUpdateFunction(t->background, noop_sprite_update);
  api->sprite->setUpdatesEnabled(t->background, false);
  api->sprite->moveTo(
    t->background, 
    col * MAP_TILE_SIZE_PX, 
    row * MAP_TILE_SIZE_PX
  );
  api->sprite->addSprite(t->background);

  t->metadata.origin.row = row;
  t->metadata.origin.col = col;
  t->metadata.shown = false;
  t->entities = vector_create(1);
}

void tile_get_metadata(tile* t, tile_metadata* tmd) {
  tmd->origin.row = t->metadata.origin.row;
  tmd->origin.col = t->metadata.origin.col;
  tmd->shown = t->metadata.shown;
}

void tile_add_entity(tile* t, entity* e) {
  vector_push(t->entities, e); 
  entity_show(e, t->metadata.shown);
}

bool entity_remove_filter(void* vector_item, void* to_remove) {
  return vector_item == to_remove;
}
void noop_cleanup(void* vector_item, void* userdata) {}

void tile_remove_entity(tile* t, entity* e) {
  int length = vector_length(t->entities);
  vector_filter(
      t->entities,
      entity_remove_filter,
      noop_cleanup,
      e
  );
  if (vector_length(t->entities) != length - 1) {
    get_api()->system->error(
      "Did not remove correct number of entities from tile"
    );
  }
}

void tile_show(tile* t, bool show) {
  if (t->metadata.shown == show) {
    return;
  }
  get_api()->sprite->setImage(
    t->background, 
    show ? white_bg : black_bg, kBitmapUnflipped
  );
  int entity_count = vector_length(t->entities);
  for (int i = 0; i < entity_count; i++) {
    entity_show((entity*)vector_item_at_index(t->entities, i), show);
  }
  t->metadata.shown = show;
}

void teardown_tile(tile* t) {
  get_api()->sprite->freeSprite(t->background);
  t->background = NULL;
  vector_destroy(t->entities);
  t->entities = NULL;
}
