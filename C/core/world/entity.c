
#include <stddef.h>
#include <string.h>

#include "C/api.h"
#include "C/core/clock.h"

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

  gid_t clock_id;

  // Used to reduce calls to memcpy and behavior.apply
  void* last_model_applied;
  int_rect last_bounds_applied;
  bool was_shown;
  bool is_game_advancing;
};

void entity_apply_changes(entity* e) {
  if (e->shown) {
    bool did_move = 
      e->bounds.x != e->last_bounds_applied.x ||
      e->bounds.y != e->last_bounds_applied.y;
    closure_call(
      e->behavior.apply, 
      e->model, 
      e->last_model_applied,
      did_move ? 1 : 0
    );

    e->model_copy(e->last_model_applied, e->model);
    memcpy(&(e->last_bounds_applied), &(e->bounds), sizeof(int_rect));
  }
  e->was_shown = e->shown;
}

static void* entity_time_advance(void* context, va_list args) {
  entity* e = (entity*)context;
  int current_time = va_arg(args, int);
  clock_mask_e clock_mask = (clock_mask_e)va_arg(args, int);
  if (clock_mask & START) {
    e->is_game_advancing = true;
  }

  if (e->behavior.plan) {
    closure_call(e->behavior.plan);
  }

  // Model changes need to be applied only once, even if multiple crank
  // ticks have passed, because only the final application will be rendered
  // to the screen and earlier applications are a waste.
  if (clock_mask & END) {
    entity_apply_changes(e);
    e->is_game_advancing = false;
  }
  return NULL;
}

entity* entity_create(
  char* label,
  point* position,
  point* size,
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
  e->bounds.x = position->x - size->x / 2;
  e->bounds.y = position->y - size->y;
  e->bounds.width = size->x;
  e->bounds.height = size->y;
  grid_pos_for_point(*position, &(e->world_pos));
  e->foothold = NULL;
  // and model state
  e->model = model_allocator();
  model_copy(e->model, model_init);

  // crank update will copy the state into beofre when the beofre is
  // needed, so initializing the remainder can be skipped.
  e->last_model_applied = model_allocator();

  memcpy(&(e->behavior), behavior, sizeof(entity_behavior));
  e->clock_id = INVALID_GID;
  e->parent_world = NULL;
  e->shown = false;
  e->was_shown = false;
  e->is_game_advancing = false;

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
  // If showing, reapply the entire current state. Skip if actively advancing,
  // apply will be called once at the end of the advancement.
  if (show && !e->is_game_advancing) {
    entity_apply_changes(e);
  }

}

void entity_get_position(entity* e, point* p) {
  p->x = e->bounds.x + e->bounds.width / 2;
  p->y = e->bounds.y + e->bounds.height;
}

void entity_get_grid_pos(entity* e, grid_pos* gp) {
  memcpy(gp, &(e->world_pos), sizeof(grid_pos));
}

void entity_get_bounds(entity* e, int_rect* b) {
  memcpy(b, &(e->bounds), sizeof(int_rect));
}

sensor* entity_get_sensor(entity* e) {
  tile* t = world_get_tile(e->parent_world, e->world_pos);
  return t->sensor;
}

void* entity_get_model(entity* e) {
  return e->model;
}

void entity_move_to(entity* e, point p) {
  point original;
  entity_get_position(e, &original);
  e->bounds.x = p.x - e->bounds.width / 2;
  e->bounds.y = p.y - e->bounds.height;
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
  e->model_destroy(e->last_model_applied);
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
  closure_call(e->behavior.spawn);
  closure_call(e->behavior.apply, e->model, NULL, 1 /* did_move */);
  e->clock_id = clock_add_listener(
    closure_create(e, entity_time_advance)
  );
}

void entity_clear_world(entity* e) {
  if (!e->parent_world) {
    get_api()->system->error("Entity does not belong to a world");
  }

  clock_remove_listener(e->clock_id);
  e->clock_id = INVALID_GID;

  closure_call(e->behavior.despawn);
  e->parent_world = NULL;
}
