#include <stdbool.h>

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/utils/closure.h"
#include "C/utils/geometry.h"

#include "tile.private.h"
#include "entity.private.h"
#include "viewport.h"

#include "world.h"

static const int VIEWPORT_TILE_SHOW_BUFFER = -1;

static const int VISIBLE_TILE_HEIGHT = 
  (SCREEN_HEIGHT_PX / MAP_TILE_SIZE_PX) + 2 * VIEWPORT_TILE_SHOW_BUFFER;
static const int VISIBLE_TILE_WIDTH = 
  (SCREEN_WIDTH_PX / MAP_TILE_SIZE_PX) + 2 * VIEWPORT_TILE_SHOW_BUFFER;

struct world_struct {
  // Tile at (row, col) is stored at (tiles_wide * row + col) tile_index
  tile*  tiles;
  int tiles_wide;
  int tiles_tall;
  int vrow;
  int vcol;
};

static int tile_index(world* w, int row, int column) {
  return row * w->tiles_wide + column;
}

world* world_create(int tiles_wide, int tiles_tall) {
  PlaydateAPI* api = get_api();
  world* w = malloc(sizeof(world));
  if (!w) {
    api->system->error("Could not allocate memory for world");
  }
  w->tiles = malloc(sizeof(tile) * tiles_wide * tiles_tall);
  if (!w->tiles) {
    api->system->error("Could not allocate memory for world tiles");
  }
  w->tiles_wide = tiles_wide;
  w->tiles_tall = tiles_tall;
  w->vrow = 0;
  w->vcol = 0;

  for (int row = 0; row < tiles_tall; row++) {
    for (int col = 0; col < tiles_wide; col++) {
      int i = tile_index(w, row, col);
      init_tile(&(w->tiles[i]), row, col);
    }
  }

  return w;
}

void world_add_entity(world* w, entity* e) {
  entity_set_world(e, w);

  point p;
  entity_get_position(e, &p);
  int col = p.x / MAP_TILE_SIZE_PX; 
  int row = p.y / MAP_TILE_SIZE_PX; 
  if (row < 0 || row >= w->tiles_tall || col < 0 || col >= w->tiles_wide) {
    get_api()->system->error("Cannot add entity outside of world bounds");
  }

  int i = tile_index(w, row, col);
  tile_add_entity(&(w->tiles[i]), e);
}

void world_remove_entity(world* w, entity* e) {
  entity_clear_world(e);

  point p;
  entity_get_position(e, &p);
  int col = p.x / MAP_TILE_SIZE_PX; 
  int row = p.y / MAP_TILE_SIZE_PX; 
  if (row < 0 || row >= w->tiles_tall || col < 0 || col > w->tiles_wide) {
    get_api()->system->error("Cannot remove entity outside of world bounds");
  }

  int i = tile_index(w, row, col);
  tile_remove_entity(&(w->tiles[i]), e);
}

void world_entity_moved(world* w, entity* e, point original) {
  point current;
  entity_get_position(e, &current);

  int old_col = original.x / MAP_TILE_SIZE_PX; 
  int old_row = original.y / MAP_TILE_SIZE_PX; 
  int new_col = current.x / MAP_TILE_SIZE_PX; 
  int new_row = current.y / MAP_TILE_SIZE_PX; 
  if (old_row == new_row && old_col == new_col) {
    return;
  }

  if (
    old_row < 0 || 
    old_row >= w->tiles_tall || 
    old_col < 0 || 
    old_col > w->tiles_wide ||
    new_row < 0 || 
    new_row >= w->tiles_tall || 
    new_col < 0 || 
    new_col > w->tiles_wide
  ) {
    get_api()->system->error("Cannot move entity outside of world bounds");
  }

  int old_index = tile_index(w, old_row, old_col);
  int new_index = tile_index(w, new_row, new_col);
  tile_remove_entity(&(w->tiles[old_index]), e);
  tile_add_entity(&(w->tiles[new_index]), e);
}
