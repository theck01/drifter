
#include "pd_api.h"

#include "C/api.h"
#include "C/core/event-emitter.h"

#include "viewport.h"

static point offset = {
  .x = 0,
  .y = 0
};
static event_emitter* emitter;

static void maybe_init(void) {
  if (!emitter) {
    emitter = event_emitter_create();
  }
}

void viewport_get_offset(point* p) {
  p->x = offset.x;
  p->y = offset.y;
}

void viewport_set_offset(int x, int y) {
  maybe_init();

  if (offset.x == x && offset.y == y) {
    return;
  }

  offset.x = x;
  offset.y = y;
  /*
   * Playdate graphics originate from the top left of the screen, with x
   * and y increasing down and right
   * Playdate screen offsets are the opposite, increasing offsets shifts
   * the screen up and left.
   * Invert the applied offset sent to Playdate so viewport offset matches
   * matches the pixel positioning.
   */
  get_api()->graphics->setDrawOffset(-x, -y);
  event_emitter_fire(emitter, x, y);
}

gid_t viewport_add_offset_listener(closure* listener) {
  maybe_init();
  return event_emitter_add(emitter, listener);
}


void viewport_remove_offset_listener(gid_t listener_id) {
  maybe_init();
  event_emitter_remove(emitter, listener_id);
}
