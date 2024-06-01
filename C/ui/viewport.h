#ifndef VIEWPORT
#define VIEWPORT

#include "C/input/controls.h"
#include "C/utils/closure.h"
#include "C/utils/geometry.h"
#include "C/utils/types.h"

void viewport_get_offset(point* p);

void viewport_set_offset(int x, int y);

/*
 * Closures:
 * listener(int x, int y): Called when the vieweport offset is moved to (x,y)
 */
gid_t viewport_add_offset_listener(closure* listener);

void viewport_remove_offset_listener(gid_t listener_id);

// Connects the viewports position to the given dpad controls
void viewport_connect(controls* c);

#endif
