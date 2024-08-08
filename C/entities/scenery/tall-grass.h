
#pragma once

#include "C/core/utils/geometry.h"
#include "C/core/world/world.h"

typedef struct tall_grass_struct tall_grass;

tall_grass* tall_grass_create(world* w, point* position);

void tall_grass_destroy(tall_grass* lg);
