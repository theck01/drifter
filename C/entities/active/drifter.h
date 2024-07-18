
#ifndef DRIFTER
#define DRIFTER

#include "C/core/controls.h"
#include "C/core/world.h"

typedef struct drifter_struct drifter;

drifter* drifter_create(
  world* w, 
  controls* c,
  point* position
);

void drifter_destroy(drifter* d);

#endif
