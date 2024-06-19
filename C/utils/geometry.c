
#include <math.h>

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"

#include "logging.h"

#include "geometry.h"

grid_pos grid_pos_for_point(point p) {
  grid_pos gp;
  gp.row = floorf((float)p.y / (float)MAP_TILE_SIZE_PX);
  gp.col = floorf((float)p.x / (float)MAP_TILE_SIZE_PX);
  return gp;
}

bool intersection(PDRect a, PDRect b, PDRect* result) {
  PlaydateAPI* api = get_api();

  float ax2 = a.x + a.width;
  float ay2 = a.y + a.height;

  float bx2 = b.x + b.width;
  float by2 = b.y + b.height;

  float ix = max(a.x, b.x);
  float ix2 = min(ax2, bx2);
  float iy = max(a.y, b.y);
  float iy2 = min(ay2, by2);

  if (ix >= ix2 || iy >= iy2) {
    return false;
  }

  result->x = ix;
  result->y = iy;
  result->width = ix2 - ix;
  result->height = iy2 - iy;
  return true;
}
