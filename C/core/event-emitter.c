
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>

#include "C/api.h"
#include "C/const.h"
#include "C/core/utils/memory/memory-recycler.h"
#include "C/core/utils/data-structures/vector.h"
#include "C/macro.h"

#include "closure.h"
#include "event-emitter.h"

struct event_emitter_struct {
  vector* listeners;
};

memory_recycler* callback_recycler = NULL;


// Callback code

typedef struct callback_struct {
  gid_t id;
  closure* closed_fn;
  bool dead;
} callback;

static void* callback_allocator(void) {
  callback* c = malloc(sizeof(callback));
  if (!c) {
    get_api()->system->error(
      "Could not allocate memory for event emitter callback"
    );
  }
  return c;
}

static void callback_destructor(void* cb) {
  callback* c = (callback*) cb;
  free(c);
}

static int8_t compare_callback_ids(void* item, void* search_id) {
  callback* c = (callback *)item;
  gid_t id = *((gid_t*) search_id);
  return c->id < id ? 1 : (c->id > id ? -1 : 0);
}

static bool callback_is_dead(void* cb, void* _) {
  return ((callback*)cb)->dead;
}

static void callback_cleanup(void* cb, void* _) {
  memory_recycler_reuse(callback_recycler, cb);
}


// Event Emitter code

static void initialize_if_needed(void) {
  if (!callback_recycler) {
    callback_recycler = memory_recycler_create(
      callback_allocator,
      callback_destructor
    );
  }
} 

event_emitter* event_emitter_create(void) {
  initialize_if_needed();

  event_emitter* e = malloc(sizeof(event_emitter));
  if (!e) {
    get_api()->system->error(
      "Could not allocate memory for event_emitter"
    );
  }

  e->listeners = vector_create(1);
  return e;
}

gid_t event_emitter_add(
  event_emitter* e, 
  closure* closed_fn
) {
  initialize_if_needed();

  callback* c = memory_recycler_get(callback_recycler);
  c->id = getNextGID();
  c->closed_fn = closed_fn;
  c->dead = false;

  vector_push(e->listeners, c);
  return c->id;
}

void event_emitter_remove(event_emitter* e,  gid_t id) {
  initialize_if_needed();

  if (id == INVALID_GID) {
    get_api()->system->error(
      "Attempting to remove an invalid event emitter ID"
    );
    return;
  }

  bsearch_result r = vector_bsearch(
    e->listeners, 
    &compare_callback_ids, 
    &id
  );
  if (!r.item) {
    get_api()->system->error(
      "Could not find event emitter listener to remove for id: %d",
      id
    );
  }

  // Do not immediately remove the listener, so that the fire method
  // can iterate through all callbacks linearly without having to keep track
  // of shifts if callbacks create or destroy one another.
  callback* c = vector_remove_at_index(e->listeners, r.index);
  c->dead = true;
}


void event_emitter_fire(event_emitter* e, ...) {
  va_list args;
  va_start(args, e);

  // Recheck listener length on each loop iteration, callbacks can create 
  // eachother (but only mark dead not destroy, so i never has to reverse)
  for (uint16_t i=0; i < vector_length(e->listeners); i++) {
    callback* c = (callback*)vector_item_at_index(e->listeners, i);
    if (!c->dead) {
      closure_vcall(c->closed_fn, args);
    }
  }

  va_end(args);

  // Recycle all dead callbacks at the end of the event firing.
  vector_filter(
    e->listeners, 
    callback_is_dead, 
    callback_cleanup, 
    NULL /*userdata */
  );
}

void event_emitter_destroy(event_emitter* e) {
  uint16_t listener_len = vector_length(e->listeners);
  for (uint16_t i = 0; i < listener_len; i++) {
    callback* c = (callback*)vector_item_at_index(e->listeners, i);
    closure_destroy(c->closed_fn);
    memory_recycler_reuse(callback_recycler, c);
  }

  vector_destroy(e->listeners);
  free(e);
}
