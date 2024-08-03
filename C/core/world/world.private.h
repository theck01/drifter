
#ifndef WORLD_PRIVATE
#define WORLD_PRIVATE

#include "tile.h"

#include "world.h"

struct world_struct {
  // Tile at (row, col) is stored at (tiles_wide * row + col) tile_index
  tile*  tiles;
  int tiles_wide;
  int tiles_tall;
  grid_pos visible_origin;
  gid_t viewport_listener_id; 
};

void world_entity_moved(world* w, entity* e, point original);

tile* world_get_tile(world* w, grid_pos p);

#endif
