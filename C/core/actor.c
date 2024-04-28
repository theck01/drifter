
#include <stddef.h>

#include "C/api.h"
#include "C/utils/history-stack.h"
#include "C/utils/memory-pool.h"

#include "crank-time.h"

#include "actor.h"

struct actor_struct {
  char* label;
  history_stack* undo;
  history_stack* redo;
  memory_pool* model_pool;
  void* current_state;
  copy_fn model_copy;
  closure* plan;
  closure* apply;
  gid_t crank_time_id;
};

static void actor_advance(actor* a) {
  void* model = history_stack_pop(a->redo);
  if (!model) {
    model = memory_pool_next(a->model_pool);
    a->model_copy(a->current_state, model);
    closure_call(a->plan, model);
  }

  closure_call(a->apply, model, a->current_state);
  history_stack_push(a->redo, model);
  a->current_state = model;
  return;
}

static void actor_reverse(actor* a) {
  void* model = history_stack_pop(a->undo);
  if (model) {
    closure_call(a->apply, model, a->current_state);
    history_stack_push(a->redo, model);
    a->current_state = model;
  }
}

static void actor_crank_update(void* context, va_list args) {
  int time_diff = va_arg(args, int);
  get_api()->system->logToConsole("Crank changed by %d ticks", time_diff);

  for (int i = time_diff; i > 0; i--) {
    actor_advance((actor*) context);
  }
  for (int i = time_diff; i < 0; i++) {
    actor_reverse((actor*) context);
  }
}

actor* actor_create(
  char* label,
  allocator_fn model_allocator,
  destructor_fn model_destructor,
  copy_fn model_copy
) {
  actor* a = malloc(sizeof(actor));
  if (!a) {
    get_api()->system->error("Could not allocate memory for actor");
  }

  a->label = label;
  a->undo = history_stack_create(HISTORY_SIZE);
  a->redo = history_stack_create(HISTORY_SIZE);
  // Additional model beyond the possible history size:
  // - Current immmutable model
  // - Modifiable upcoming model
  a->model_pool = memory_pool_create(
    HISTORY_SIZE + 2, 
    model_allocator, 
    model_destructor
  );
  a->current_state = memory_pool_next(a->model_pool);
  a->model_copy = model_copy;
  a->plan = NULL;
  a->apply = NULL;
  a->crank_time_id = INVALID_GID;

  return a;
}

char* actor_get_label(actor* a) {
  return a->label;
}

void actor_start_updates(
  actor* a, 
  closure* init, 
  closure* plan, 
  closure* apply
) {
  if (a->crank_time_id != INVALID_GID) {
    get_api()->system->error("Actor is already receiving updates");
  }

  a->plan = plan;
  a->apply = apply;

  closure_call(init, a->current_state);
  closure_destroy(init);

  closure_call(apply, a->current_state, NULL);

  a->crank_time_id = crank_time_add_listener(
    closure_create(a, actor_crank_update)
  );
}

void actor_stop_updates(actor* a) {
  if (a->crank_time_id == INVALID_GID) {
    get_api()->system->error("Actor has already stopped updates");
  }
  crank_time_remove_listener(a->crank_time_id);
  a->crank_time_id = INVALID_GID;
  closure_destroy(a->plan);
  a->plan = NULL;
  closure_destroy(a->apply);
  a->apply = NULL;
}

void actor_destroy(actor* a) {
  if (a->crank_time_id != INVALID_GID) {
    actor_stop_updates(a);
  }

  a->current_state = NULL;
  memory_pool_destroy(a->model_pool);
  a->model_pool = NULL;

  history_stack_destroy(a->undo);
  a->undo = NULL;
  history_stack_destroy(a->redo);
  a->redo = NULL;

  a->model_copy = NULL;
}
