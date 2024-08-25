
#ifndef SENSOR
#define SENSOR

#include <stdint.h>
#include <stdbool.h>

#include "C/core/utils/geometry.h"
#include "C/core/utils/types.h"

#include "types.h"

sensor* sensor_create(uint8_t tile_radius, grid_pos center, world* w);

entity* sensor_get_entity_nearest(sensor* s, entity* e);

// sensor will not allow moving outside of tile radius, among other conditions
// dictated by world state such as world boundary, or solid tiles, etc.
// Returns false if actual differs from desired, otherwise true
bool sensor_can_entity_move(sensor* s, entity* e, point desired, point* actual);

void sensor_destroy(sensor* s);

#endif
