
#include "pd_api.h"

#include "C/api.h"

#include "viewport.h"

static point offset = {
  .x = 0,
  .y = 0
};

void viewport_get_offset(point* p) {
  p->x = offset.x;
  p->y = offset.y;
}

void viewport_set_offset(int x, int y) {
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
}
