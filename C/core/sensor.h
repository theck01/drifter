
#ifndef SENSOR
#define SENSOR

#include <stdint.h>
#include <stdbool.h>

#include "types.h"

#include "C/utils/geometry.h"
#include "C/utils/types.h"

sensor* sensor_create(uint8_t tile_radius, grid_pos center, world* w);

// sensor will not allow moving outside of tile radius, among other conditions
// dictated by world state such as world boundary, or solid tiles, etc.
// Returns false if actual differs from desired, otherwise true
bool sensor_can_entity_move(sensor* s, entity* e, point desired, point* actual);

void sensor_destroy(sensor* s);

#endif
