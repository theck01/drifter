
#ifndef UTIL_GEOMETRY
#define UTIL_GEOMETRY

#include <stdbool.h>

#include "pd_api.h"

#include "types.h"

typedef struct point_struct {
  int x;
  int y;
} point;

grid_pos grid_pos_for_point(point p);

bool intersection(PDRect a, PDRect b, PDRect* result);

#endif
