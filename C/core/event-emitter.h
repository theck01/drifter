
#ifndef EVENT_EMITTER
#define EVENT_EMITTER

#include <stdint.h>

#include "C/core/utils/types.h"

#include "closure.h"


typedef struct event_emitter_struct event_emitter;

event_emitter* event_emitter_create(void);

// Closure will be called with arguments corresponding to event data.
// See individual event creators (fps_timers, crank_time) for information
// on what data format to expect.
gid_t event_emitter_add(event_emitter* e, closure* c);

void event_emitter_remove(event_emitter* e, gid_t listener_id);

void event_emitter_fire(event_emitter* e, ...);

void event_emitter_destroy(event_emitter* e);

#endif
