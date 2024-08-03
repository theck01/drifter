
#ifndef TILE_PRIVATE
#define TILE_PRIVATE

#include "C/utils/vector.h"

#include "sensor.h"

#include "tile.h"

struct tile_struct {
  tile_metadata metadata;
  vector* entities;
  LCDSprite* background;
  sensor* sensor;
};

void init_tile(tile* t, int row, int col, world* w);

void teardown_tile(tile* t);

#endif
