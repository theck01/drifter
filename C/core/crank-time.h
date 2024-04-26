
#ifndef CRANK_TIME
#define CRANK_TIME

#include "C/utils/closure.h"
#include "C/utils/types.h"

/*
 * Closures:
 * listener(int8_t time_diff): Called when the crank causes a time delta
 */
gid_t crank_time_add_listener(closure* listener);

void crank_time_remove_listener(gid_t id);

void crank_time_update(void);

#endif
