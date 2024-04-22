
#ifndef EVENT_EMITTER
#define EVENT_EMITTER

#include <stdint.h>

#include "C/utils/closure.h"
#include "C/utils/types.h"

typedef struct event_emitter_struct event_emitter;

event_emitter* event_emitter_create(void);

// Closure will be called with (void* eventdata) as an argument in addition
// to its own context.
gid_t event_emitter_add(event_emitter* e, closure* c);

void event_emitter_remove(event_emitter* e, gid_t listener_id);

void event_emitter_fire(event_emitter* e, void* eventdata);

void event_emitter_destroy(event_emitter* e);

#endif
