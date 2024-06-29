#include <stdbool.h>

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/utils/closure.h"
#include "C/utils/geometry.h"

#include "tile.private.h"
#include "entity.private.h"
#include "viewport.h"

#include "world.private.h"

static const int VIEWPORT_TILE_SHOW_BUFFER = -1;

static const int VISIBLE_TILE_HEIGHT = 
  (SCREEN_HEIGHT_PX / MAP_TILE_SIZE_PX) + 2 * VIEWPORT_TILE_SHOW_BUFFER;
static const int VISIBLE_TILE_WIDTH = 
  (SCREEN_WIDTH_PX / MAP_TILE_SIZE_PX) + 2 * VIEWPORT_TILE_SHOW_BUFFER;

static int tile_index(world* w, int row, int column) {
  return row * w->tiles_wide + column;
}

// Differentiate between callers that want -VIEWPORT_TILE_SHOW_BUFFER as origin,
// and 0 as origin
static int world_clamp_row(world* w, int row) {
  return clamp(row, 0, w->tiles_tall);
}

static int world_clamp_col(world* w, int col) {
  return clamp(col, 0, w->tiles_wide);
}

static void* viewport_moved(void* world_to_cast, va_list args) {
  world* w = world_to_cast; 
  point p = { .x = va_arg(args, int), .y = va_arg(args, int) };
  grid_pos prev_visible = w->visible_origin;
  point show_point = { 
    .x = p.x - MAP_TILE_SIZE_PX * VIEWPORT_TILE_SHOW_BUFFER,
    .y = p.y - MAP_TILE_SIZE_PX * VIEWPORT_TILE_SHOW_BUFFER
  };
  grid_pos_for_point(show_point, &(w->visible_origin));

  if (
    w->visible_origin.row != prev_visible.row || 
    w->visible_origin.col != prev_visible.col
  ) {
    int row_delta = w->visible_origin.row - prev_visible.row;
    int hide_row_start = -1, hide_row_end = -1;
    int show_row_start = -1, show_row_end = -1;
    if (row_delta > 0) {
      hide_row_start = prev_visible.row;
      hide_row_end = hide_row_start + min(row_delta, VISIBLE_TILE_HEIGHT);
      show_row_end = w->visible_origin.row + VISIBLE_TILE_HEIGHT;
      show_row_start = show_row_end - min(row_delta, VISIBLE_TILE_HEIGHT);
    } else if (row_delta < 0) {
      hide_row_end = prev_visible.row + VISIBLE_TILE_HEIGHT;
      hide_row_start = hide_row_end + max(row_delta, -VISIBLE_TILE_HEIGHT);
      show_row_start = w->visible_origin.row;
      show_row_end = show_row_start - max(row_delta, -VISIBLE_TILE_HEIGHT);
    }

    hide_row_start = world_clamp_row(w, hide_row_start);
    hide_row_end = world_clamp_row(w, hide_row_end);
    show_row_start = world_clamp_row(w, show_row_start);
    show_row_end = world_clamp_row(w, show_row_end);

    // Hide rows based on previous viewport tile position
    for (int row = hide_row_start; row < hide_row_end; row++) {
      int col_start = world_clamp_col(w, prev_visible.col);
      int col_end = world_clamp_col(w, prev_visible.col + VISIBLE_TILE_WIDTH);
      for (int col = col_start; col < col_end; col++) {
        int i = tile_index(w, row, col);
        tile_show(&(w->tiles[i]), false);
      }
    }

    // Show rows based on new viewport tile position
    for (int row = show_row_start; row < show_row_end; row++) {
      int col_start = world_clamp_col(w, w->visible_origin.col);
      int col_end = world_clamp_col(
          w, 
          w->visible_origin.col + VISIBLE_TILE_WIDTH
      );
      for (int col = col_start; col < col_end; col++) {
        int i = tile_index(w, row, col);
        tile_show(&(w->tiles[i]), true);
      }
    }

    int col_delta = w->visible_origin.col - prev_visible.col;
    int hide_col_start = -1, hide_col_end = -1;
    int show_col_start = -1, show_col_end = -1;
    if (col_delta > 0) {
      hide_col_start = prev_visible.col;
      hide_col_end = hide_col_start + min(col_delta, VISIBLE_TILE_WIDTH);
      show_col_end = w->visible_origin.col + VISIBLE_TILE_WIDTH;
      show_col_start = show_col_end - min(col_delta, VISIBLE_TILE_WIDTH);
    } else if (col_delta < 0) {
      hide_col_end = prev_visible.col + VISIBLE_TILE_WIDTH;
      hide_col_start = hide_col_end + max(col_delta, -VISIBLE_TILE_WIDTH);
      show_col_start = w->visible_origin.col;
      show_col_end = show_col_start - max(col_delta, -VISIBLE_TILE_WIDTH);
    }

    hide_col_start = world_clamp_col(w, hide_col_start);
    hide_col_end = world_clamp_col(w, hide_col_end);
    show_col_start = world_clamp_col(w, show_col_start);
    show_col_end = world_clamp_col(w, show_col_end);

    // Hide columns based on previous viewport tile position
    int row_start = world_clamp_row(w, prev_visible.row);
    int row_end = world_clamp_row(w, prev_visible.row + VISIBLE_TILE_HEIGHT);
    for (int row = row_start; row < row_end; row++) {
      for (int col = hide_col_start; col < hide_col_end; col++) {
        int i = tile_index(w, row, col);
        tile_show(&(w->tiles[i]), false);
      }
    }

    // Show columns based on new viewport tile position
    row_start = world_clamp_row(w, w->visible_origin.row);
    row_end = world_clamp_row(w, w->visible_origin.row + VISIBLE_TILE_HEIGHT);
    for (int row = row_start; row < row_end; row++) {
      for (int col = show_col_start; col < show_col_end; col++) {
        int i = tile_index(w, row, col);
        tile_show(&(w->tiles[i]), true);
      }
    }
  }

  return NULL;
}

world* world_create(int tiles_wide, int tiles_tall) {
  PlaydateAPI* api = get_api();
  world* w = malloc(sizeof(world));
  if (!w) {
    api->system->error("Could not allocate memory for world");
  }
  w->tiles = malloc(sizeof(tile) * tiles_wide * tiles_tall);
  if (!w->tiles) {
    api->system->error("Could not allocate memory for world tiles");
  }
  w->tiles_wide = tiles_wide;
  w->tiles_tall = tiles_tall;

  for (int row = 0; row < tiles_tall; row++) {
    for (int col = 0; col < tiles_wide; col++) {
      int i = tile_index(w, row, col);
      init_tile(&(w->tiles[i]), row, col, w);
      // Show the tiles as if the viewport is at 0,0 initially, so viewport
      // update has an initial state to diff from
      if (
        row >= -VIEWPORT_TILE_SHOW_BUFFER && 
        row < VISIBLE_TILE_HEIGHT - VIEWPORT_TILE_SHOW_BUFFER &&
        col >= -VIEWPORT_TILE_SHOW_BUFFER && 
        col < VISIBLE_TILE_WIDTH - VIEWPORT_TILE_SHOW_BUFFER
      ) {
        tile_show(&(w->tiles[i]), true);
      }
    }
  }
  w->visible_origin.row = -1;
  w->visible_origin.col = -1;

  point v;
  viewport_get_offset(&v);
  closure* viewport_listener = closure_create(w, viewport_moved);
  closure_call(viewport_listener, v.x, v.y);
  w->viewport_listener_id = viewport_add_offset_listener(viewport_listener);

  return w;
}

void world_add_entity(world* w, entity* e) {
  entity_set_world(e, w);

  grid_pos gp;
  entity_get_grid_pos(e, &gp);
  if (
    gp.row < 0 || 
    gp.row >= w->tiles_tall || 
    gp.col < 0 || 
    gp.col >= w->tiles_wide
  ) {
    get_api()->system->error("Cannot add entity outside of world bounds");
  }

  int i = tile_index(w, gp.row, gp.col);
  tile_add_entity(&(w->tiles[i]), e);
}

void world_remove_entity(world* w, entity* e) {
  entity_clear_world(e);

  grid_pos gp;
  entity_get_grid_pos(e, &gp);
  if (
    gp.row < 0 || 
    gp.row >= w->tiles_tall || 
    gp.col < 0 || 
    gp.col >= w->tiles_wide
  ) {
    get_api()->system->error("Cannot remove entity outside of world bounds");
  }

  int i = tile_index(w, gp.row, gp.col);
  tile_remove_entity(&(w->tiles[i]), e);
}

void world_entity_moved(world* w, entity* e, point original) {
  grid_pos old;
  grid_pos_for_point(original, &old);
  grid_pos new;
  entity_get_grid_pos(e, &new);
  if (old.row == new.row && old.col == new.col) {
    return;
  }

  if (
    old.row < 0 || 
    old.row >= w->tiles_tall || 
    old.col < 0 || 
    old.col > w->tiles_wide ||
    new.row < 0 || 
    new.row >= w->tiles_tall || 
    new.col < 0 || 
    new.col > w->tiles_wide
  ) {
    get_api()->system->error("Cannot move entity outside of world bounds");
  }

  int old_index = tile_index(w, old.row, old.col);
  int new_index = tile_index(w, new.row, new.col);
  tile_remove_entity(&(w->tiles[old_index]), e);
  tile_add_entity(&(w->tiles[new_index]), e);
}

tile* world_get_tile(world* w, grid_pos p) { 
  if (
    p.row >= w->tiles_tall || 
    p.row < 0 || 
    p.col >= w->tiles_wide || 
    p.col < 0
  ) {
    return NULL;
  }

  int i = tile_index(w, p.row, p.col);
  return &(w->tiles[i]);
}

