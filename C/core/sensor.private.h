
#ifndef SENSOR_PRIVATE
#define SENSOR_PRIVATE

#include "C/utils/types.h"

#include "tile.private.h"

#include "sensor.h"

struct sensor_struct {
  grid_pos center;
  uint8_t tile_radius;
  tile** tile_refs;
};

#endif
