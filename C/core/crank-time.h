
#ifndef CRANK_TIME
#define CRANK_TIME

#include "C/utils/closure.h"
#include "C/utils/types.h"

// Closure is called with a int8_t representing number of time steps to
// advance or retreat.
gid_t crank_time_add_listener(closure* listener);

void crank_time_remove_listener(gid_t id);

void crank_time_update(void);

#endif
