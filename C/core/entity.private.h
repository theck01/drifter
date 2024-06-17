#ifndef ENTITY_PRIVATE
#define ENTITY_PRIVATE

#include "world.h"
#include "entity.h"

void entity_show(entity* e, bool show);

void entity_set_world(entity* e,  world* w);

void entity_clear_world(entity* e);

#endif
