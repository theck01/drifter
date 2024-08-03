
#ifndef WORLD
#define WORLD

#include "C/utils/geometry.h"

#include "types.h"

world* world_create(int tiles_wide, int tiles_tall);

void world_add_entity(world* w, entity* e);

void world_remove_entity(world* w, entity* e);

#endif
