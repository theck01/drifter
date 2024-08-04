
#ifndef CLOCK
#define CLOCK

#include "C/core/utils/types.h"

#include "closure.h"

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
gid_t clock_add_listener(closure* listener);

void clock_remove_listener(gid_t id);

void clock_update(void);

#endif
