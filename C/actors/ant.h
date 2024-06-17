
#ifndef ANT_ACTOR
#define ANT_ACTOR

#include "C/core/world.h"

typedef struct ant_struct ant;

ant* ant_create(world* w, int x, int y);

void ant_destroy(ant* ant);

#endif
