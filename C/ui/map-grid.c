#include <limits.h>
#include <math.h>
#include <stdbool.h>

#include "pd_api.h"

#include "C/api.h"
#include "C/const.h"
#include "C/utils/geometry.h"

#include "viewport.h"

#include "map-grid.h"

static LCDBitmap* grid_img = NULL;
static LCDSprite* grid = NULL;
bool shown = false;
point grid_offset = {
  .x = -(MAP_TILE_SIZE_PX/2),
  .y = -(MAP_TILE_SIZE_PX/2)
};
point last_viewport_offset = {
  .x = INT_MAX,
  .y = INT_MAX
};

void position_grid(LCDSprite* s) {
  point viewport_offset;
  viewport_get_offset(&viewport_offset);

  if (
    last_viewport_offset.x == viewport_offset.x &&
    last_viewport_offset.y == viewport_offset.y
  ) {
    return;    
  }

  last_viewport_offset.x = viewport_offset.x;
  last_viewport_offset.y = viewport_offset.y;

  int xshift = 0;
  if (viewport_offset.x < grid_offset.x) {
    xshift = ceilf((viewport_offset.x - grid_offset.x) / (float)MAP_TILE_SIZE_PX);
  } else if (viewport_offset.x > grid_offset.x + MAP_TILE_SIZE_PX) {
    xshift = ceilf(
        (viewport_offset.x - (grid_offset.x + MAP_TILE_SIZE_PX)) / 
        (float)MAP_TILE_SIZE_PX
    );
  }

  int yshift = 0;
  if (viewport_offset.y < grid_offset.y) {
    yshift = ceilf((viewport_offset.y - grid_offset.y) / (float)MAP_TILE_SIZE_PX);
  } else if (viewport_offset.y > grid_offset.y + MAP_TILE_SIZE_PX) {
    yshift = ceilf(
        (viewport_offset.y - (grid_offset.y + MAP_TILE_SIZE_PX)) / 
        (float)MAP_TILE_SIZE_PX
    );
  }

  if (xshift || yshift) {
    get_api()->sprite->moveTo(
      grid,
      grid_offset.x + (xshift * MAP_TILE_SIZE_PX), 
      grid_offset.y + (yshift * MAP_TILE_SIZE_PX)
    );
  }
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
    api->sprite->setUpdateFunction(grid, &position_grid);
    api->sprite->moveTo(grid, grid_offset.x, grid_offset.y);
    api->sprite->addSprite(grid);
  }
  api->sprite->setVisible(grid, 1);
  shown = true;
}

void map_grid_hide(void) {
  PlaydateAPI* api = get_api();

  if (!shown) {
    api->system->error("Map grid is already hidden");
    return;
  }

  api->sprite->setVisible(grid, 0);
  shown = false;
}
