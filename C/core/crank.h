
#ifndef CRANK
#define CRANK

#include "C/utils/closure.h"
#include "C/utils/types.h"

/*
 * Closures:
 * listener(
 *   int tick_diff
 * ): Called when the cranking causes a change in tick count
 */
gid_t crank_add_listener(closure* listener);

void crank_remove_listener(gid_t id);

void crank_check(void);

#endif
