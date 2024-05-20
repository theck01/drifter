#include <math.h>

#include "pd_api.h"

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/core/crank-time.h"
#include "C/utils/closure.h"
#include "C/utils/geometry.h"
#include "C/utils/types.h"

#include "history-gauge.h"

// Foreground Relative Bounds (FRB) vs background
const PDRect FRB = { .x = 4, .y = 4, .width = 4, .height = 232 };

typedef struct gauge_struct {
  uint16_t ticks_remaining;
  uint16_t potential_size;
  LCDSprite* background;
  LCDSprite* foreground;
  gid_t crank_id;
} gauge;

static gauge history_gauge = {
  .ticks_remaining = 0,
  .potential_size = 0,
  .background = NULL,
  .foreground = NULL,
  .crank_id = INVALID_GID
};
LCDBitmap* background_img;

void noop_update(LCDSprite* s) {}

void draw_gauge_bar(LCDSprite* s, PDRect bounds, PDRect dirty_rect) {
  PlaydateAPI* api = get_api();
  int bar_height = floorf(
    (bounds.height / HISTORY_SIZE) * history_gauge.ticks_remaining
  );

  PDRect fill_rect = {
    .x = bounds.x,
    .y = bounds.y + bounds.height - bar_height,
    .width = bounds.width,
    .height = bar_height
  };
  PDRect dirty_fill;
  if (intersection(dirty_rect, fill_rect, &dirty_fill)) {
    api->graphics->fillRect(
      dirty_fill.x,
      dirty_fill.y,
      dirty_fill.width,
      dirty_fill.height,
      kColorWhite
    );
  }

  PDRect clear_rect = {
    .x = bounds.x,
    .y = bounds.y,
    .width = bounds.width,
    .height = bounds.height - bar_height
  };
  PDRect dirty_clear;
  if (intersection(dirty_rect, clear_rect, &dirty_clear)) {
    api->graphics->fillRect(
      dirty_clear.x,
      dirty_clear.y,
      dirty_clear.width,
      dirty_clear.height,
      kColorClear
    );
  }
}

void load_sprites(void) {
  PlaydateAPI* api = get_api();

  const char* err;
  background_img = api->graphics->loadBitmap(
    "img/history-gauge-background.png",
    &err
  );
  if (!background_img) {
    api->system->error(
      "Could not load history gauge background image, err: %s", 
      err
    );
  }

  history_gauge.background = api->sprite->newSprite();
  api->sprite->setCenter(history_gauge.background, 0, 0);
  api->sprite->setZIndex(history_gauge.background, HUD_Z_INDEX);
  api->sprite->setImage(
    history_gauge.background, 
    background_img, 
    kBitmapUnflipped
  );
  api->sprite->setUpdateFunction(history_gauge.background, &noop_update);

  history_gauge.foreground = api->sprite->newSprite();
  api->sprite->setCenter(history_gauge.foreground, 0, 0);
  api->sprite->setZIndex(history_gauge.foreground, HUD_Z_INDEX);
  api->sprite->setSize(history_gauge.foreground, FRB.width, FRB.height);
  api->sprite->setDrawFunction(history_gauge.foreground, &draw_gauge_bar);
  api->sprite->setUpdateFunction(history_gauge.foreground, &noop_update);
}

static void gauge_crank_update(void* gauge, va_list args) {
  int time_diff = va_arg(args, int);

  history_gauge.ticks_remaining = min(
    max(history_gauge.ticks_remaining + time_diff, 0), 
    HISTORY_SIZE
  );
  history_gauge.potential_size = max(
    history_gauge.ticks_remaining, 
    history_gauge.potential_size
  );

  get_api()->sprite->markDirty(history_gauge.foreground);
}

void history_gauge_connect(void) {
  PlaydateAPI* api = get_api();

  if (history_gauge.crank_id != INVALID_GID) {
    api->system->error("History gauge already connected to crank");
  }

  if (!history_gauge.background || !history_gauge.foreground) {
    load_sprites();
  }

  api->sprite->moveTo(history_gauge.background, 400 - 12, 0);
  api->sprite->addSprite(history_gauge.background);

  api->sprite->moveTo(
    history_gauge.foreground, 
    400 - 12 + FRB.x, 
    FRB.y
  );
  api->sprite->addSprite(history_gauge.foreground);

  history_gauge.crank_id = crank_time_add_listener(
    closure_create(&history_gauge, gauge_crank_update)
  );
}

void history_gauge_disconnect(void) {
  PlaydateAPI* api = get_api();
  if (history_gauge.crank_id != INVALID_GID) {
    get_api()->system->error("History gauge already connected to crank");
  }

  crank_time_remove_listener(history_gauge.crank_id);
  history_gauge.crank_id = INVALID_GID;
  api->sprite->removeSprite(history_gauge.background);
  api->sprite->removeSprite(history_gauge.foreground);
}
