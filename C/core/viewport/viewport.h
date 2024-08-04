#ifndef VIEWPORT
#define VIEWPORT

#include "C/core/closure.h"
#include "C/core/utils/geometry.h"
#include "C/core/utils/types.h"

void viewport_get_offset(point* p);

void viewport_set_offset(int x, int y);

/*
 * Closures:
 * listener(int x, int y): Called when the vieweport offset is moved to (x,y)
 */
gid_t viewport_add_offset_listener(closure* listener);

void viewport_remove_offset_listener(gid_t listener_id);

#endif
