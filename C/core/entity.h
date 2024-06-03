
#ifndef ENTITY
#define ENTITY

#include "C/const.h"
#include "C/utils/closure.h"
#include "C/utils/types.h"

typedef struct entity_struct entity;

/*
 * Creates the memory needed for the entity, but does not start running its
 * update loop
 */
entity* entity_create(
  char* label,
  allocator_fn model_allocator,
  destructor_fn model_destructor,
  copy_fn model_copy
);

char* entity_get_label(entity* e);

/*
 * Closures:
 * init(void* model): Initialize the entity's first model, called immediately
 * apply(void* model, void* prev_model): Update any secondary effects to match
 *   model. prev_model may be NULL if there is no previous valid state
 */
void entity_start_passive(
  entity* e, 
  closure* init, 
  closure* apply
);

/*
 * Closures:
 * Both of the above, plus
 * plan(void* model_to_update, current_model): Update the argument model to 
 *   advance one time unit. Both models are set to same state at beginning,
 *   model_to_update is the only one that should be modified.
 */
void entity_start_active(
  entity* e, 
  closure* init, 
  closure* apply,
  closure* plan
);

void entity_stop_updates(entity* e);

void entity_destroy(entity* e);

#endif
