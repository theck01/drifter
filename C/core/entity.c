
#include <stddef.h>
#include <string.h>

#include "C/api.h"
#include "C/core/crank-time.h"

#include "world.private.h"
#include "tile.private.h"
#include "structure.h"

#include "entity.private.h"

struct entity_struct {
  char* label;

  int_rect bounds;
  grid_pos world_pos;
  bool shown;

  world* parent_world;
  structure* foothold;

  void* model;
  destructor_fn model_destroy;
  copy_fn model_copy;

  entity_behavior behavior;

  gid_t crank_time_id;

  // Used to reduce calls to memcpy and behavior.apply
  void* model_before_crank;
  int_rect bounds_before_crank;
  bool was_shown;
  bool is_cranking;
};

static void* entity_crank_advance(void* context, va_list args) {
  entity* e = (entity*)context;
  int current_time = va_arg(args, int);
  crank_mask_e crank_mask = (crank_mask_e)va_arg(args, int);
  if (crank_mask & START) {
    e->is_cranking = true;
    e->was_shown=e->shown;
    // Only do a moderately expensive model copy if the entity is shown,
    // Hidden entites initialize secondary effects at the end of crankiing, with
    // NULL prior state
    if (e->shown) {
      memcpy(&(e->bounds_before_crank), &(e->bounds), sizeof(int_rect));
      e->model_copy(e->model_before_crank, e->model);
    }
  }

  if (e->behavior.plan) {
    // Assume the tile exists, because the entity was allowed to exist in the
    // current position.
    tile* t = world_get_tile(e->parent_world, e->world_pos);
    closure_call(e->behavior.plan, e->model, t->sensor);
  }

  // Model changes need to be applied only once, even if multiple crank
  // ticks have passed, because only the final application will be rendered
  // to the screen and earlier applications are a waste.
  if (crank_mask & END) {
    if (e->shown) {
      bool did_move = 
          !e->was_shown || 
          e->bounds.x != e->bounds_before_crank.x ||
          e->bounds.y != e->bounds_before_crank.y;
      closure_call(
        e->behavior.apply, 
        e->model, 
        e->was_shown ? e->model_before_crank : NULL,
        did_move ? 1 : 0
      );
    }
    e->is_cranking = false;
  }
  return NULL;
}

entity* entity_create(
  char* label,
  int_rect* bounds,
  void* model_init,
  entity_behavior* behavior,
  allocator_fn model_allocator,
  destructor_fn model_destructor,
  copy_fn model_copy
) {
  entity* e = malloc(sizeof(entity));
  if (!e) {
    get_api()->system->error("Could not allocate memory for entity");
  }

  e->label = label;


  e->model_destroy = model_destructor;
  e->model_copy = model_copy;

  // Setup core model
  memcpy(&(e->bounds), bounds, sizeof(int_rect));
  point p = { .x = bounds->x, .y = bounds->y };
  grid_pos_for_point(p, &(e->world_pos));
  e->foothold = NULL;
  // and model state
  e->model = model_allocator();
  model_copy(e->model, model_init);

  // crank update will copy the state into before_crank when the before_crank is
  // needed, so initializing the remainder can be skipped.
  e->model_before_crank = model_allocator();

  memcpy(&(e->behavior), behavior, sizeof(entity_behavior));
  e->crank_time_id = INVALID_GID;
  e->parent_world = NULL;
  e->shown = false;
  e->was_shown = false;
  e->is_cranking = false;

  return e;
}

char* entity_get_label(entity* e) {
  return e->label;
}

void entity_show(entity* e, bool show) {
  if (!e->parent_world) {
    get_api()->system->error(
      "Cannot change the visibility of a worldless entity"
    );
  }

  if (e->shown == show) {
    return;
  }
  e->shown = show;
  closure_call(e->behavior.show, show ? 1 : 0);
  // If showing, reapply the entire current state. Skip if actively cranking,
  // apply will be called once at the end of the crank processing.
  if (show && !e->is_cranking) {
    closure_call(e->behavior.apply, e->model, NULL);
  }
}

void entity_get_position(entity* e, point* p) {
  p->x = e->bounds.x;
  p->y = e->bounds.y;
}

void entity_get_grid_pos(entity* e, grid_pos* gp) {
  memcpy(gp, &(e->world_pos), sizeof(grid_pos));
}

void entity_get_bounds(entity* e, int_rect* b) {
  memcpy(b, &(e->bounds), sizeof(int_rect));
}

void entity_move_to(entity* e, point p) {
  point original = { .x =e->bounds.x, .y = e->bounds.y };
  e->bounds.x = p.x;
  e->bounds.y = p.y;
  grid_pos_for_point(p, &e->world_pos);
  if (e->parent_world) {
    world_entity_moved(e->parent_world, e, original);
  }
}

void entity_destroy(entity* e) {
  if (e->parent_world) {
    get_api()->system->error("Remove entity from world before destroying");
  }

  closure_destroy(e->behavior.spawn);
  e->behavior.spawn = NULL;
  closure_destroy(e->behavior.despawn);
  e->behavior.despawn = NULL;
  closure_destroy(e->behavior.show);
  e->behavior.show = NULL;
  closure_destroy(e->behavior.apply);
  e->behavior.apply = NULL;
  closure_destroy(e->behavior.plan);
  e->behavior.plan = NULL;

  e->model_destroy(e->model);
  e->model_destroy(e->model_before_crank);
  e->model_destroy = NULL;
  e->model_copy = NULL;
}

void entity_set_world(entity* e, world* w) {
  if (e->parent_world) {
    get_api()->system->error("Entity already belongs to a world");
  }

  if (!e->behavior.apply) {
    get_api()->system->error("Cannot add entity without behavior to world");
  }

  e->parent_world = w;
  closure_call(e->behavior.spawn, e->model);
  e->crank_time_id = crank_time_advance_listener(
    closure_create(e, entity_crank_advance)
  );
}

void entity_clear_world(entity* e) {
  if (!e->parent_world) {
    get_api()->system->error("Entity does not belong to a world");
  }

  crank_time_remove_listener(e->crank_time_id);
  e->crank_time_id = INVALID_GID;

  closure_call(e->behavior.despawn);
  e->parent_world = NULL;
}
