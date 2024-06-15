#include <limits.h>
#include <math.h>
#include <stdbool.h>

#include "pd_api.h"

#include "C/api.h"
#include "C/const.h"
#include "C/core/viewport.h"
#include "C/utils/closure.h"
#include "C/utils/functions.h"
#include "C/utils/geometry.h"
#include "C/utils/types.h"


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

    grid = api->sprite->newSprite();
    api->sprite->setCenter(grid, 0, 0);
    api->sprite->setZIndex(grid, MAP_GRID_Z_INDEX);
    api->sprite->setImage(grid, grid_img, kBitmapUnflipped);
    api->sprite->setUpdateFunction(grid, noop_sprite_update);
    api->sprite->moveTo(grid, grid_offset.x, grid_offset.y);
    api->sprite->addSprite(grid);
  }

  api->sprite->setVisible(grid, 1);
  viewport_listener_id = viewport_add_offset_listener(
    closure_create(NULL /* context */, viewport_moved)
  );
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
