
#include <stddef.h>

#include "C/api.h"
#include "C/core/crank-time.h"
#include "C/utils/history-stack.h"
#include "C/utils/memory-pool.h"

#include "entity.h"

struct entity_struct {
  char* label;
  history_stack* undo;
  history_stack* redo;
  memory_pool* model_pool;
  void* current_model;
  copy_fn model_copy;
  closure* plan;
  closure* apply;
  gid_t crank_time_id;
};

static void entity_advance(entity* e) {
  void* model = history_stack_pop(e->redo);
  if (!model) {
    model = memory_pool_next(e->model_pool);
    e->model_copy(e->current_model, model);
    if (e->plan) {
      closure_call(e->plan, model, e->current_model);
    }
  }

  closure_call(e->apply, model, e->current_model);
  history_stack_push(e->undo, model);
  e->current_model = model;
  return;
}

static void entity_reverse(entity* e) {
  void* model = history_stack_pop(e->undo);
  if (model) {
    closure_call(e->apply, model, e->current_model);
    history_stack_push(e->redo, model);
    e->current_model = model;
  }
}

static void entity_crank_update(void* context, va_list args) {
  int time_diff = va_arg(args, int);
  for (int i = time_diff; i > 0; i--) {
    entity_advance((entity*) context);
  }
  for (int i = time_diff; i < 0; i++) {
    entity_reverse((entity*) context);
  }
}

entity* entity_create(
  char* label,
  allocator_fn model_allocator,
  destructor_fn model_destructor,
  copy_fn model_copy
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
    model_allocator, 
    model_destructor
  );
  e->current_model = memory_pool_next(e->model_pool);
  e->model_copy = model_copy;
  e->plan = NULL;
  e->apply = NULL;
  e->crank_time_id = INVALID_GID;

  return e;
}

char* entity_get_label(entity* e) {
  return e->label;
}

void entity_start_passive(
  entity* e,
  closure* init, 
  closure* apply
) {
  entity_start_active(e, init, apply, NULL /* plan */);
}

void entity_start_active(
  entity* e, 
  closure* init, 
  closure* apply,
  closure* plan 
) {
  if (e->crank_time_id != INVALID_GID) {
    get_api()->system->error("Entity is already receiving updates");
  }

  e->plan = plan;
  e->apply = apply;

  closure_call(init, e->current_model);
  closure_destroy(init);

  closure_call(apply, e->current_model, NULL);

  e->crank_time_id = crank_time_add_listener(
    closure_create(e, entity_crank_update)
  );
}

void entity_stop_updates(entity* e) {
  if (e->crank_time_id == INVALID_GID) {
    get_api()->system->error("Entity has already stopped updates");
  }
  crank_time_remove_listener(e->crank_time_id);
  e->crank_time_id = INVALID_GID;
  closure_destroy(e->plan);
  e->plan = NULL;
  closure_destroy(e->apply);
  e->apply = NULL;
}

void entity_destroy(entity* e) {
  if (e->crank_time_id != INVALID_GID) {
    entity_stop_updates(e);
  }

  e->current_model = NULL;
  memory_pool_destroy(e->model_pool);
  e->model_pool = NULL;

  history_stack_destroy(e->undo);
  e->undo = NULL;
  history_stack_destroy(e->redo);
  e->redo = NULL;

  e->model_copy = NULL;
}
