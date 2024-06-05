
#ifndef ENTITY
#define ENTITY

#include "C/const.h"
#include "C/utils/closure.h"
#include "C/utils/geometry.h"
#include "C/utils/types.h"

typedef struct entity_struct entity;

typedef struct entity_state_struct {
  point position;
} entity_state;

typedef struct entity_model_struct {
  entity_state core;
  void* extended;
} entity_model;

typedef struct entity_base_behavior_struct {
  /*
   * apply(entity_model* model, entity_model* prev_model): Update any secondary
   *   effects to match model. prev_model may be NULL if there is no previous 
   *   valid state.
   *
   *   Most commonly updating sprites or sounds to reflect the model change.
   */
  closure* apply;
  /*
   * show(bool should_show): Show or hide secondary effects such as sprites
   *   sound effects, most commonly for when objects are positioned far off
   *   screen.
   *
   *   `apply` will not be called while hidden.
   */
  closure* show;
} entity_base_behavior;

typedef struct entity_active_behavior_struct {
  // From base above
  closure* apply;
  closure* show;

  /*
   * plan(entity_model* model_to_update, entity_model* current_model): Update 
   *   the argument model to advance one time unit. Both models are set to same
   *   state at beginning model_to_update is the only one that should be 
   *   modified.
   */
  closure* plan;
} entity_active_behavior;

/*
 * Creates the memory needed for the entity, but does not start running its
 * update loop
 *
 * Properties will be copied from the initial model, entity will not take
 * ownership.
 */
entity* entity_create(
  char* label,
  allocator_fn model_allocator,
  destructor_fn model_destructor,
  copy_fn model_copy,
  entity_model* init 
);

char* entity_get_label(entity* e);

/* Takes ownership of behavior closures, but not the parent object */
void entity_start(entity* e, entity_base_behavior* behavior);
void entity_start_active(entity* e, entity_active_behavior* behavior);

void entity_stop_updates(entity* e);

void entity_show(entity* e, bool show);

void entity_destroy(entity* e);

#endif
