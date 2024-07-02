
#ifndef CRANK_TIME
#define CRANK_TIME

#include "C/utils/closure.h"
#include "C/utils/types.h"

// The crank can generate more than one tick per frame, the listener call for
// the first tick will have the START mask, the listener call for the last
// tick will have the END mask. If only one tick was advanced that frame, then
// both START and END masks will be present
typedef enum {
  START = 0b01,
  END = 0b10,
} crank_mask_e;

/*
 * Closures:
 * listener(
 *   int current_time,
 *   crank_mask_e,
 * ): Called when the cranking causes time to change
 */
gid_t crank_time_advance_listener(closure* listener);

void crank_time_remove_listener(gid_t id);

void crank_time_update(void);

#endif
