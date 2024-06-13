
#include <stddef.h>

#include "C/api.h"
#include "C/core/crank-time.h"
#include "C/utils/history-stack.h"
#include "C/utils/memory-pool.h"

#include "world.h"

#include "entity.private.h"

typedef struct entity_full_behavior {
  closure* apply;
  closure* show;
  closure* plan;
} entity_full_behavior;

struct entity_struct {
  char* label;
  history_stack* undo;
  history_stack* redo;
  memory_pool* model_pool;
  entity_model* current_model;
  closure* model_copy;
  entity_full_behavior behavior;
  gid_t crank_time_id;
  world* parent_world;
  bool shown;
};

static void entity_set_current_model(entity* e, entity_model* model) {
  point old_position = e->current_model->core.position;

  if (e->shown) {
    closure_call(e->behavior.apply, model, e->current_model);
  }

  // Update entity model before notifying world it moved, so that the world
  // update uses correct positions.
  e->current_model = model;
  if (
    e->parent_world && 
    (model->core.position.x != old_position.x ||
    model->core.position.y != old_position.y)
  ) {
    world_entity_moved(e->parent_world, e, old_position);
  }
}

static void entity_advance(entity* e) {
  entity_model* model = (entity_model*)history_stack_pop(e->redo);
  if (!model) {
    model = (entity_model*)memory_pool_next(e->model_pool);
    closure_call(e->model_copy, e->current_model, model);
    if (e->behavior.plan) {
      closure_call(e->behavior.plan, model, e->current_model);
    }
  }
  history_stack_push(e->undo, model);
  entity_set_current_model(e, model);
  return;
}

static void entity_reverse(entity* e) {
  entity_model* model = history_stack_pop(e->undo);
  if (model) {
    history_stack_push(e->redo, model);
    entity_set_current_model(e, model);
  }
}

static void* entity_crank_update(void* context, va_list args) {
  int time_diff = va_arg(args, int);
  for (int i = time_diff; i > 0; i--) {
    entity_advance((entity*) context);
  }
  for (int i = time_diff; i < 0; i++) {
    entity_reverse((entity*) context);
  }
  return NULL;
}

void* entity_model_allocator(void* extended_allocator, va_list _) {
  entity_model* model = malloc(sizeof(entity_model));
  if (!model) {
    get_api()->system->error("Could not allocate memory for entity model");
  }
  allocator_fn ea = (allocator_fn)extended_allocator;
  model->extended = ea();
  return model;
}

void* entity_model_destructor(void* extended_destructor, va_list args) {
  entity_model* model = va_arg(args, entity_model*);
  destructor_fn ed = (destructor_fn)extended_destructor;
  ed(model->extended);
  free(model);
  return NULL;
}

void* entity_model_copy(void* extended_copy, va_list args) {
  entity_model* source = va_arg(args, entity_model*);
  entity_model* destination = va_arg(args, entity_model*);
  copy_fn ec = (copy_fn)extended_copy;

  destination->core.position.x = source->core.position.x;
  destination->core.position.y = source->core.position.y;
  ec(source->extended, destination->extended);

  return NULL;
}


entity* entity_create(
  char* label,
  allocator_fn extended_model_allocator,
  destructor_fn extended_model_destructor,
  copy_fn extended_model_copy,
  entity_model* init
) {
  entity* e = malloc(sizeof(entity));
  if (!e) {
    get_api()->system->error("Could not allocate memory for entity");
  }

  e->label = label;
  e->undo = history_stack_create(HISTORY_SIZE);
  e->redo = history_stack_create(HISTORY_SIZE);
  // Additional model beyond the possible history size:
  // - Current immmutable model
  // - Modifiable upcoming model
  e->model_pool = memory_pool_create(
    HISTORY_SIZE + 2, 
    closure_create(
      extended_model_allocator,
      entity_model_allocator
    ),
    closure_create(
      extended_model_destructor,
      entity_model_destructor
    )
  );
  e->current_model = (entity_model*)memory_pool_next(e->model_pool);
  e->model_copy = closure_create(extended_model_copy, entity_model_copy);
  closure_call(e->model_copy, init, e->current_model);

  e->behavior.apply = NULL;
  e->behavior.show = NULL;
  e->behavior.plan = NULL;

  e->crank_time_id = INVALID_GID;
  e->parent_world = NULL;
  e->shown = false;

  return e;
}

char* entity_get_label(entity* e) {
  return e->label;
}

void entity_cleanup_behavior(entity* e) {
  if (e->behavior.show) {
    closure_destroy(e->behavior.show);
    e->behavior.show = NULL;
  }
  if (e->behavior.apply) {
    closure_destroy(e->behavior.apply);
    e->behavior.apply = NULL;
  }
  if (e->behavior.plan) {
    closure_destroy(e->behavior.plan);
    e->behavior.plan = NULL;
  }
}

void entity_set_behavior(entity* e, entity_base_behavior behavior) {
  entity_cleanup_behavior(e);
  e->behavior.show = behavior.show;
  e->behavior.apply = behavior.apply;
}

void entity_set_active(entity* e, entity_active_behavior behavior) {
  entity_cleanup_behavior(e);
  e->behavior.show = behavior.base.show;
  e->behavior.apply = behavior.base.apply;
  e->behavior.plan = behavior.plan;
}

void entity_show(entity* e, bool show) {
  if (e->shown == show) {
    return;
  }
  e->shown = show;
  closure_call(e->behavior.show, show);
  // If showing, reapply the entire current state.
  if (show) {
    closure_call(e->behavior.apply, e->current_model, NULL);
  }
}

void entity_get_position(entity* e, point* p) {
  p->x = e->current_model->core.position.x;
  p->y = e->current_model->core.position.y;
}

void entity_destroy(entity* e) {
  if (e->parent_world) {
    get_api()->system->error("Remove entity from world before destroying");
  }

  e->current_model = NULL;
  memory_pool_destroy(e->model_pool);
  e->model_pool = NULL;

  history_stack_destroy(e->undo);
  e->undo = NULL;
  history_stack_destroy(e->redo);
  e->redo = NULL;

  entity_cleanup_behavior(e);

  closure_destroy(e->model_copy);
  e->model_copy = NULL;
}

void entity_set_world(entity* e, world* w) {
  if (e->parent_world) {
    get_api()->system->error("Entity already belongs to a world");
  }
  if (!e->behavior.apply) {
    get_api()->system->error("Cannot add entity without behavior to world");
  }

  e->crank_time_id = crank_time_add_listener(
    closure_create(e, entity_crank_update)
  );
  e->parent_world = w;
  closure_call(e->behavior.apply, e->current_model, NULL);
}

void entity_clear_world(entity* e) {
  if (!e->parent_world) {
    get_api()->system->error("Entity does not belong to a world");
  }
  crank_time_remove_listener(e->crank_time_id);
  e->crank_time_id = INVALID_GID;
  e->parent_world = NULL;
}
