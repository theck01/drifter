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


// CONST

// Bacground Bounds (BB)
const PDRect BB = { .x = 388, .y = 0, .width = 12, .height = 240 };
// Foreground Relative Bounds (FRB) vs background
const PDRect FRB = { .x = 4, .y = 4, .width = 4, .height = 232 };

const float CRANK_ANIMATION_OFFSETS[] = {
  16,
  16,
  16,
  11,
  5,
  1,
  -1,
  -3,
  -4,
  -3,
  -1,
  0
};
const uint8_t CRANK_ANIMATION_DURATION = 12;


// DATA TYPES

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


// SPRITE FNS

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


// HISTORY GAUGE

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
  api->sprite->setVisible(history_gauge.background, 0);
  api->sprite->addSprite(history_gauge.background);

  history_gauge.foreground = api->sprite->newSprite();
  api->sprite->setCenter(history_gauge.foreground, 0, 0);
  api->sprite->setZIndex(history_gauge.foreground, HUD_Z_INDEX);
  api->sprite->setSize(history_gauge.foreground, FRB.width, FRB.height);
  api->sprite->setDrawFunction(history_gauge.foreground, &draw_gauge_bar);
  api->sprite->setUpdateFunction(history_gauge.foreground, &noop_update);
  api->sprite->setVisible(history_gauge.foreground, 0);
  api->sprite->addSprite(history_gauge.foreground);
}

static void gauge_crank_update(void* gauge, va_list args) {
  PlaydateAPI* api = get_api();
  int time_diff = va_arg(args, int);
  bool was_showing = 
    history_gauge.ticks_remaining < history_gauge.potential_size;
  int past_delta = 
    history_gauge.potential_size - history_gauge.ticks_remaining;

  history_gauge.ticks_remaining = min(
    max(history_gauge.ticks_remaining + time_diff, 0), 
    HISTORY_SIZE
  );
  history_gauge.potential_size = max(
    history_gauge.ticks_remaining, 
    history_gauge.potential_size
  );

  int delta = 
    history_gauge.potential_size - history_gauge.ticks_remaining;
  if (delta) {
    if (!was_showing) {
      api->sprite->setVisible(history_gauge.background, 1);
      api->sprite->moveTo(
        history_gauge.background, 
        BB.x + CRANK_ANIMATION_OFFSETS[0], 
        0
      );

      api->sprite->setVisible(history_gauge.foreground, 1);
      api->sprite->moveTo(
        history_gauge.foreground, 
        BB.x + FRB.x  + CRANK_ANIMATION_OFFSETS[0], 
        FRB.y
      );
    }

    if (delta < CRANK_ANIMATION_DURATION) {
      api->sprite->moveTo(
        history_gauge.background, 
        BB.x + CRANK_ANIMATION_OFFSETS[delta], 
        0
      );
      api->sprite->moveTo(
        history_gauge.foreground, 
        BB.x + FRB.x + CRANK_ANIMATION_OFFSETS[delta], 
        FRB.y
      );
    } else if (past_delta < CRANK_ANIMATION_DURATION) {
      api->sprite->moveTo(history_gauge.background, BB.x, 0);
      api->sprite->moveTo(history_gauge.foreground, BB.x + FRB.x, FRB.y);
    }
  } else {
    if (was_showing) {
      api->sprite->setVisible(history_gauge.background, 0);
      api->sprite->setVisible(history_gauge.foreground, 0);
    }
  }

  if (delta || past_delta) {
    api->sprite->markDirty(history_gauge.foreground);
  }
}

void history_gauge_connect(void) {
  PlaydateAPI* api = get_api();

  if (history_gauge.crank_id != INVALID_GID) {
    api->system->error("History gauge already connected to crank");
  }

  if (!history_gauge.background || !history_gauge.foreground) {
    load_sprites();
  }

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
  api->sprite->setVisible(history_gauge.background, 0);
  api->sprite->setVisible(history_gauge.foreground, 0);
}
