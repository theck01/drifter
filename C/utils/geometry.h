
#ifndef UTIL_GEOMETRY
#define UTIL_GEOMETRY

#include <stdbool.h>

#include "pd_api.h"

typedef struct point_struct {
  int x;
  int y;
} point;

bool intersection(PDRect a, PDRect b, PDRect* result);

#endif
