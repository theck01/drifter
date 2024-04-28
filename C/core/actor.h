
#ifndef ACTOR
#define ACTOR

#include "C/const.h"
#include "C/utils/closure.h"
#include "C/utils/types.h"

typedef struct actor_struct actor;

/*
 * Creates the memory needed for the actor, but does not start running its
 * update loop
 */
actor* actor_create(
  char* label,
  allocator_fn model_allocator,
  destructor_fn model_destructor,
  copy_fn model_copy
);

char* actor_get_label(actor* a);

/*
 * Closures:
 * init(void* model): Initialize the actor's first model, called immediately
 * plan(void* model_to_update, current_model): Update the argument model to 
 *   advance one time unit. Both models are set to same state at beginning,
 *   model_to_update is the only one that should be modified.
 * apply(void* model, void* prev_model): Update any secondary effects to match
 *   model. prev_model may be NULL if there is no previous valid state
 */
void actor_start_updates(
  actor* a, 
  closure* init, 
  closure* plan, 
  closure* apply
);

void actor_stop_updates(actor* a);

void actor_destroy(actor* a);

#endif
