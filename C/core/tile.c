
#include "C/api.h"

#include "entity.private.h"

#include "tile.private.h"

void init_tile(tile* t, int row, int col) {
  t->metadata.row = row;
  t->metadata.col = col;
  t->metadata.shown = true;
  t->entities = vector_create(1);
}

void tile_get_metadata(tile* t, tile_metadata* tmd) {
  tmd->row = t->metadata.row;
  tmd->col = t->metadata.col;
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
  for (int i = vector_length(t->entities) - 1; i >= 0; i++) {
    entity_show((entity*)vector_item_at_index(t->entities, i), show);
  }
  t->metadata.shown = true;
}

void teardown_tile(tile* t) {
  vector_destroy(t->entities);
}
