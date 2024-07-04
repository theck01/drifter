
#ifndef GAME_CLOCK
#define GAME_CLOCK

#include "C/utils/closure.h"
#include "C/utils/types.h"

// The game can generate more than one tick per frame, the listener call for
// the first tick will have the START mask, the listener call for the last
// tick will have the END mask. If only one tick was advanced that frame, then
// both START and END masks will be present
typedef enum {
  START = 0b01,
  END = 0b10,
} clock_mask_e;

/*
 * Closures:
 * listener(
 *   int current_time,
 *   clock_mask_e,
 * ): Called when the gameing causes time to change
 */
gid_t game_clock_add_listener(closure* listener);

void game_clock_remove_listener(gid_t id);

void game_clock_update(void);

#endif
