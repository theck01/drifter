
#include <stddef.h>
#include <math.h>

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"

#include "logging.h"

#include "geometry.h"

void grid_pos_for_point(point p, grid_pos* gp) {
  gp->row = p.y / MAP_TILE_SIZE_PX - (p.y < 0 ? 1 : 0); 
  gp->col = p.x / MAP_TILE_SIZE_PX - (p.x < 0 ? 1 : 0); 
}

void math_vec_init(math_vec* v, point start, point end) {
  memcpy(&v->start, &start, sizeof(point));
  memcpy(&v->end, &end, sizeof(point));
  v->magnitude = sqrtf(powf(end.x - start.x, 2) + powf(end.y - start.y, 2));
}

void math_to_unit_vec(math_vec v, unit_vec* u) {
  u->x = (v.end.x - v.start.x) / v.magnitude;
  u->y = (v.end.y - v.start.y) / v.magnitude;
}

void math_vec_unit_normal(math_vec v, unit_vec* u, point pointing_away_from) {
  float a = v.end.x - v.start.x;
  float b = v.end.y - v.start.y;
  u->x = -b / v.magnitude;
  u->y = a / v.magnitude;

  // Dot product the new unit vector with a pseudovector from point_away_from to
  // either end v. If the result is positive, the unit vector is correctly
  // pointing away from the point. If the result is negative, invert its x and y
  // to correct the vector direction
  float pseudo_dot = 
    (v.end.x - pointing_away_from.x) * u->x + 
    (v.end.y - pointing_away_from.y) * u->y;
  if (pseudo_dot < 0) {
    u->x *= -1;
    u->y *= -1;
  }
}

bool int_intersection(int_rect a, int_rect b, int_rect* result) {
  PlaydateAPI* api = get_api();

  int ax2 = a.x + a.width;
  int ay2 = a.y + a.height;

  int bx2 = b.x + b.width;
  int by2 = b.y + b.height;

  int ix = max(a.x, b.x);
  int ix2 = min(ax2, bx2);
  int iy = max(a.y, b.y);
  int iy2 = min(ay2, by2);

  if (ix >= ix2 || iy >= iy2) {
    return false;
  }

  result->x = ix;
  result->y = iy;
  result->width = ix2 - ix;
  result->height = iy2 - iy;
  return true;
}

bool pd_intersection(PDRect a, PDRect b, PDRect* result) {
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
