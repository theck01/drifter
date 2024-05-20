
#include "C/api.h"
#include "C/macro.h"

#include "logging.h"

#include "geometry.h"

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
