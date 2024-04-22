
#include "C/api.h"

#include "memory-recycler.h"

#include "closure.h"

struct closure_struct {
  void* context;
  closure_fn fn;
  closure_context_cleanup_fn cleanup_fn;
};
static memory_recycler* closure_recycler = NULL;

static void* closure_allocator(void) {
  closure* c = malloc(sizeof(closure));
  if (!c) {
    get_api()->system->error("Could not allocate memory for closure");
  }
  return c;
}

static void closure_destructor(void* closed_fn) {
  closure* c = (closure*) closed_fn;
  free(c);
}

static void initialize_if_needed(void) {
  if (!closure_recycler) {
    closure_recycler = memory_recycler_create(
      closure_allocator, 
      closure_destructor
    );
  }
}

closure* closure_create(void* context, closure_fn fn, closure_context_cleanup_fn cleanup_fn) {
  initialize_if_needed();
  closure* c = (closure*) memory_recycler_get(closure_recycler);
  c->context = context;
  c->fn = fn;
  c->cleanup_fn = cleanup_fn;
  return c;
}


void closure_call(closure* c, ...) {
  va_list args;
  va_start(args, c);
  c->fn(c->context, args);
  va_end(args);
}

void closure_destroy(closure* c) {
  if (c->cleanup_fn) {
    c->cleanup_fn(c->context);
  }
  c->context = NULL;
  c->fn = NULL;
  c->cleanup_fn = NULL;
  memory_recycler_reuse(closure_recycler, c);
}
