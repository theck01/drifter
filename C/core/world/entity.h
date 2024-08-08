
#ifndef ENTITY
#define ENTITY

#include "C/const.h"
#include "C/core/closure.h"
#include "C/core/utils/geometry.h"
#include "C/core/utils/types.h"

#include "types.h"

typedef struct entity_behavior_struct {
  /*
   * spawn(void): Generate any sprites/sounds associated with the entity.
   * Called on addition to the world.
   */
  closure* spawn;
  /*
   * apply(void* extended_model, void* extended_model, int did_move): Update any
   *   secondary effects to match model. prev_model may be NULL if there is no
   *   previous valid state.
   *
   *   Sprite positions only need be changed if did_move is 1, otherwise will
   *   be 0.
   *
   *   Most commonly updating sprites or sounds to reflect the model change.
   */
  closure* apply;
  /*
   * show(int should_show): Show or hide secondary effects such as sprites
   *   sound effects, most commonly for when objects are positioned far off
   *   screen.
   *
   *   `apply` will not be called while hidden.
   */
  closure* show;
  /*
   * despawn(void): Remove any sprite/sounds associated with the entity. Called
   * when it is removed from the world
   */
  closure* despawn;
  /*
   * plan(void) Update the entity to advance one time unit.
   */
  closure* plan;
} entity_behavior;

/*
 * Creates the memory needed for the entity, but does not start running its
 * update loop
 *
 * Properties will be copied from the initial model, entity will not take
 * ownership.
 */
entity* entity_create(
  const char* label,
  const point* position,
  const point* size,
  const void* model_init, 
  const entity_behavior* behavior,
  allocator_fn model_allocator,
  destructor_fn model_destructor,
  copy_fn model_copy
);

const char* entity_get_label(entity* e);

void entity_get_position(entity* e, point* p);
void entity_get_grid_pos(entity* e, grid_pos* gp);
void entity_get_bounds(entity* e, int_rect* b);

sensor* entity_get_sensor(entity* e);

void entity_move_to(entity* e, point p);

void* entity_get_model(entity* e);

void entity_destroy(entity* e);

#endif
