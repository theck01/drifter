
#include <stdbool.h>
#include <math.h>

#include "C/api.h"
#include "C/utils/closure.h"

#include "crank.h"
#include "event-emitter.h"

#include "game-clock.h"

static event_emitter* emitter = NULL;

static int current_time = 0;

static void initialize_if_needed(void) {
  if (!emitter) {
    emitter = event_emitter_create();
  }
}

gid_t game_clock_add_listener(closure* c) {
  initialize_if_needed();
  return event_emitter_add(emitter, c);
}

void game_clock_remove_listener(gid_t id) {
  initialize_if_needed();
  event_emitter_remove(emitter, id);
}

void game_clock_update(void) {
  initialize_if_needed();
  event_emitter_fire(emitter, ++current_time, START | END);
}
