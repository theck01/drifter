
#ifndef DRIFTER
#define DRIFTER

#include "C/core/controls/controls.h"
#include "C/core/world/world.h"

typedef struct drifter_struct drifter;

drifter* drifter_create(
  world* w, 
  controls* c,
  point* position
);

entity* drifter_get_entity(drifter* d);

void drifter_destroy(drifter* d);

#endif
