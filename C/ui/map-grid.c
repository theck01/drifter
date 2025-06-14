#include <limits.h>
#include <math.h>
#include <stdbool.h>

#include "pd_api.h"

#include "C/api.h"
#include "C/const.h"
#include "C/core/closure.h"
#include "C/core/graphics/sprite.h"
#include "C/core/utils/geometry.h"
#include "C/core/utils/types.h"
#include "C/core/viewport/viewport.h"


#include "map-grid.h"

static LCDBitmap* grid_img = NULL;
static LCDSprite* grid = NULL;
gid_t viewport_listener_id = INVALID_GID;
bool shown = false;
point grid_offset = {
  .x = -(MAP_TILE_SIZE_PX/2),
  .y = -(MAP_TILE_SIZE_PX/2)
};

void* viewport_moved(void* _, va_list args) {
  int vx = va_arg(args, int);
  int vy = va_arg(args, int);

  int xshift = 0;
  if (vx < grid_offset.x) {
    xshift = floorf((vx - grid_offset.x) / (float)MAP_TILE_SIZE_PX);
  } else if (vx > grid_offset.x + MAP_TILE_SIZE_PX) {
    xshift = ceilf(
        (vx - (grid_offset.x + MAP_TILE_SIZE_PX)) / 
        (float)MAP_TILE_SIZE_PX
    );
  }

  int yshift = 0;
  if (vy < grid_offset.y) {
    yshift = floorf((vy - grid_offset.y) / (float)MAP_TILE_SIZE_PX);
  } else if (vy > grid_offset.y + MAP_TILE_SIZE_PX) {
    yshift = ceilf(
        (vy - (grid_offset.y + MAP_TILE_SIZE_PX)) / 
        (float)MAP_TILE_SIZE_PX
    );
  }

  if (xshift || yshift) {
    grid_offset.x += (xshift * MAP_TILE_SIZE_PX);
    grid_offset.y += (yshift * MAP_TILE_SIZE_PX);
    get_api()->sprite->moveTo(grid, grid_offset.x, grid_offset.y);
  }

  return NULL;
}

void map_grid_show(void) {
  PlaydateAPI* api = get_api();

  if (shown) {
    api->system->error("Map grid is already shown");
    return;
  }

  if (!grid) {
    const char* err;
    grid_img = api->graphics->loadBitmap(
      "img/map-grid.png",
      &err
    );
    if (!grid_img) {
      api->system->error(
        "Could not load map grid image, err: %s", 
        err
      );
    }

    grid = create_draw_only_sprite();
    api->sprite->setCenter(grid, 0, 0);
    api->sprite->setZIndex(grid, MAP_GRID_Z_INDEX);
    api->sprite->setImage(grid, grid_img, kBitmapUnflipped);
    api->sprite->moveTo(grid, grid_offset.x, grid_offset.y);
    api->sprite->addSprite(grid);
  }

  closure* viewport_moved_listener = 
    closure_create(NULL /* context */, viewport_moved);

  point offset;
  viewport_get_offset(&offset);
  closure_call(viewport_moved_listener, offset.x, offset.y);
  viewport_listener_id = viewport_add_offset_listener(
      viewport_moved_listener
  );

  api->sprite->setVisible(grid, 1);
  shown = true;
}

void map_grid_hide(void) {
  PlaydateAPI* api = get_api();

  if (!shown) {
    api->system->error("Map grid is already hidden");
    return;
  }
  viewport_remove_offset_listener(viewport_listener_id);
  viewport_listener_id = INVALID_GID;
  api->sprite->setVisible(grid, 0);
  shown = false;
}
