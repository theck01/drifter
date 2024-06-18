
#include "C/api.h"
#include "C/const.h"

#include "tile.private.h"
#include "sensor.private.h"
#include "world.private.h"

static tile* tile_for_grid_pos(sensor* s, grid_pos p) {
  int sensor_view_size = s->tile_radius * 2 + 1;
  grid_pos origin = { 
    .row = s->center.row - s->tile_radius, 
    .col = s->center.col - s->tile_radius
  };

  if (
    p.row >= origin.row && p.row < origin.row + sensor_view_size &&
    p.col >= origin.col && p.col < origin.col + sensor_view_size
  ) {
    int i = (p.row - origin.row) * sensor_view_size + (p.col - origin.col);
    tile* t = s->tile_refs[i];
    if (
      t && (t->metadata.origin.row != p.row || t->metadata.origin.col != p.col)
    ) {
      get_api()->system->error(
        "Sensor has incorrectly positioned tile, expected (r %d, c %d) but got (r %d, c %d)", 
        p.row, p.col, 
        t->metadata.origin.row, 
        t->metadata.origin.col
      );
    }
    return t;
  }
  return NULL;
}

sensor* sensor_create(uint8_t tile_radius, grid_pos center, world* w) {
  sensor* s = malloc(sizeof(sensor));
  if (!s) {
    get_api()->system->error("Could not allocate memory for sensor");
  }
  s->center = center;
  s->tile_radius = tile_radius;


  int i = 0;
  int sensor_view_size = tile_radius * 2 + 1;
  s->tile_refs = malloc(sizeof(tile*) * sensor_view_size * sensor_view_size); 
  for (
    int row = center.row - tile_radius; 
    row <= center.row + tile_radius; 
    row++
  ) {
    for (
      int col = center.col - tile_radius;
      col <= center.col + tile_radius;
      col++
    ) {
      int row_diff = center.row > row ? center.row - row : row - center.row;
      int col_diff = center.col > col ? center.col - col : col - center.col;
      if (row_diff + col_diff <= tile_radius) {
        grid_pos p = { .row = row, .col = col };
        s->tile_refs[i] = world_get_tile(w, p);
      } else {
        s->tile_refs[i] = NULL;
      }
      i++;
    }
  }

  return s;
}

bool sensor_can_entity_move(
  sensor* s, 
  entity* e, 
  point desired, 
  point* actual
) {
  grid_pos desired_pos = {
   .col = desired.x / MAP_TILE_SIZE_PX,
   .row = desired.y / MAP_TILE_SIZE_PX
  };
  // Overly simple early implementation, this needs to consider tile state and
  // tile entities as well as the existance of a tile at that position.
  tile* t = tile_for_grid_pos(s, desired_pos);
  return t != NULL;

}

void sensor_destroy(sensor* s) {
  free(s->tile_refs);
  free(s);
}
