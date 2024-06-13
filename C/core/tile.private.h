
#ifndef TILE_PRIVATE
#define TILE_PRIVATE

#include "C/utils/vector.h"

#include "tile.h"

struct tile_struct {
  tile_metadata metadata;
  vector* entities;
};

void init_tile(tile* t, int row, int col);

void teardown_tile(tile* t);

#endif
