
#ifndef STRUCTURE
#define STRUCTURE

#include "C/utils/geometry.h"

#include "entity.h"

typedef struct structure_struct structure;

structure* structure_create(
  point center,
  // Expected to be a closed, convex shape when connecting the final perimeter
  // point to the start
  const point* perimeter, 
  uint8_t point_count,
  /*
   * apply(point* position, point* prev_position): Update any secondary
   *   effects to match a structrue movement. prev_position may be NULL if there
   *   is no prior positon. 
   *
   *   Most commonly updating sprites or sounds to reflect structure movement
   */
  closure* apply
);

bool structure_attach(structure* s, entity* e);
void structure_detatch(structure*s, entity* e);

// Positive distances go from start to end, negative from end to start
void structure_move_along_surface(structure* s, entity* e, int distance);

void structure_destroy(structure* s);

// Logs all information about the structure to the console
void structure_log(structure* s);

#endif
