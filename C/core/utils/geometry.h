
#ifndef UTIL_GEOMETRY
#define UTIL_GEOMETRY

#include <stdbool.h>

#include "pd_api.h"

#include "types.h"

typedef struct point_struct {
  int x;
  int y;
} point;

typedef struct int_rect_struct {
  int x;
  int y;
  unsigned int width;
  unsigned int height;
} int_rect;

void int_rect_to_lcd(int_rect* ir, LCDRect* lcd);

void grid_pos_for_point(point p, grid_pos* gp);

typedef struct math_vec_struct {
  point start;
  point end;
  float magnitude;
} math_vec;

void math_vec_init(math_vec* v, point start, point end);

typedef struct unit_vec_struct {
  float x;
  float y;
} unit_vec;

void math_to_unit_vec(math_vec v, unit_vec* u);

// A vector has two normals, one going out in either direction from the vector
// perpendicularly.
//
// The pointing_away_from point ensures that the correct of the two possible
// normal vectors is calculated. Often this point would be the center of 
// a structure with surface normal vectors all point outward from the structure.
void math_vec_unit_normal(math_vec v, unit_vec* u, point pointing_away_from);

bool int_intersection(int_rect a, int_rect b, int_rect* result);
bool pd_intersection(PDRect a, PDRect b, PDRect* result);

#endif
