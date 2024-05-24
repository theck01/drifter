#include <math.h>

#include "pd_api.h"

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/core/crank-time.h"
#include "C/utils/closure.h"
#include "C/utils/dither.h"
#include "C/utils/functions.h"
#include "C/utils/geometry.h"
#include "C/utils/random.h"
#include "C/utils/types.h"

#include "history-gauge.h"


// CONST

// Background Bounds (BB)
const PDRect BB = { .x = 388, .y = 0, .width = 12, .height = 240 };
// Gauge Relative Bounds (GRB) vs background
const PDRect GRB = { .x = 4, .y = 4, .width = 4, .height = 232 };

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

typedef enum {
  IDLE = 0,
  RETURNING = 1,
  LEAVING = 2
} vibration_e;



// DATA TYPES

typedef struct gauge_struct {
  uint16_t ticks_remaining;
  uint16_t potential_size;
  vibration_e vibration;
  int sprite_animation_frame;
  int sprite_animation_increment;
  LCDSprite* background;
  LCDSprite* unused_capacity;
  // This must be the last sprite added for correct ZIndex, and also for correct
  // sprite animation for all sprites that make up the gauge.
  LCDSprite* foreground;
  gid_t crank_id;
} gauge;

static gauge history_gauge = {
  .ticks_remaining = 0,
  .potential_size = 0,
  .vibration = IDLE,
  .sprite_animation_frame = -1,
  .sprite_animation_increment = 1,
  .background = NULL,
  .unused_capacity = NULL,
  .foreground = NULL,
  .crank_id = INVALID_GID
};
LCDBitmap* background_img;


// SPRITE FNS

// Gauge positioning *mostly* happens in this function, logic is split
// with the crank time listener where a vibration and animation info
// is setup.
void position_sprites(LCDSprite* s) {
  PlaydateAPI* api = get_api();

  // If the sprite is animating into or out of position, update positioning
  // to match animation frame.
  int animation_frame = history_gauge.sprite_animation_frame;
  int increment = history_gauge.sprite_animation_increment;
  if (animation_frame > -1) {
    // On the first frame of the show animation, or the last frame of the hide
    // animation, update sprite visibility to match
    if (animation_frame == 0) {
      int show = history_gauge.sprite_animation_increment > 0 ? 1 : 0;
      api->sprite->setVisible(history_gauge.background, show);
      api->sprite->setVisible(history_gauge.unused_capacity, show);
      api->sprite->setVisible(history_gauge.foreground, show);
    }

    api->sprite->moveTo(
      history_gauge.background,
      BB.x + CRANK_ANIMATION_OFFSETS[animation_frame], 
      0
    );
    api->sprite->moveTo(
      history_gauge.unused_capacity,
      BB.x + GRB.x  + CRANK_ANIMATION_OFFSETS[animation_frame], 
      GRB.y
    );
    api->sprite->moveTo(
      history_gauge.foreground,
      BB.x + GRB.x  + CRANK_ANIMATION_OFFSETS[animation_frame], 
      GRB.y
    );

    int next_frame = animation_frame + increment;
    if ((next_frame < 0 || next_frame >= CRANK_ANIMATION_DURATION)) {
      history_gauge.sprite_animation_frame = -1;
      history_gauge.sprite_animation_increment = 0;
    } else {
      history_gauge.sprite_animation_frame = next_frame;
    }
  }

  // Correct potential vibration animation for depleted history
  vibration_e next = IDLE;
  if (history_gauge.vibration == RETURNING) {
    api->sprite->moveTo(history_gauge.background, BB.x, 0);
    api->sprite->moveTo(history_gauge.foreground, BB.x + GRB.x, GRB.y);
    api->sprite->moveTo(history_gauge.unused_capacity, BB.x + GRB.x, GRB.y);
  } else if (history_gauge.vibration == LEAVING) {
    int xshift = random_uint(0, 4) - 2;      
    int yshift = random_uint(0, 4) - 2;      
    api->sprite->moveTo(history_gauge.background, BB.x + xshift, yshift);
    api->sprite->moveTo(
      history_gauge.unused_capacity, 
      BB.x + GRB.x + xshift, 
      GRB.y + yshift
    );
    api->sprite->moveTo(
      history_gauge.foreground, 
      BB.x + GRB.x + xshift, 
      GRB.y + yshift
    );
    next = RETURNING;
  }
  history_gauge.vibration = next;
}

int calc_height_within(int history_idx, PDRect bounds) {
  return floorf((bounds.height / HISTORY_SIZE) * history_idx);
}

void draw_gauge_bar(LCDSprite* s, PDRect bounds, PDRect dirty_rect) {
  PlaydateAPI* api = get_api();
  int bar_height = calc_height_within(history_gauge.ticks_remaining, bounds);

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

void draw_unused_capacity(LCDSprite* s, PDRect bounds, PDRect dirty_rect) {
  PlaydateAPI* api = get_api();
  int bar_height = calc_height_within(
    (HISTORY_SIZE - history_gauge.potential_size),
    bounds
  );

  PDRect fill_rect = {
    .x = bounds.x,
    .y = bounds.y,
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
      DITHER_5050
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
  api->sprite->setUpdateFunction(
    history_gauge.background, 
    &position_sprites
  );
  api->sprite->setVisible(history_gauge.background, 0);
  api->sprite->addSprite(history_gauge.background);

  history_gauge.unused_capacity = api->sprite->newSprite();
  api->sprite->setCenter(history_gauge.unused_capacity, 0, 0);
  api->sprite->setZIndex(history_gauge.unused_capacity, HUD_Z_INDEX);
  api->sprite->setSize(history_gauge.unused_capacity, GRB.width, GRB.height);
  api->sprite->setDrawFunction(history_gauge.unused_capacity, &draw_unused_capacity);
  api->sprite->setUpdateFunction(
    history_gauge.unused_capacity, 
    &noop_sprite_update
  );
  api->sprite->setVisible(history_gauge.unused_capacity, 0);
  api->sprite->addSprite(history_gauge.unused_capacity);

  history_gauge.foreground = api->sprite->newSprite();
  api->sprite->setCenter(history_gauge.foreground, 0, 0);
  api->sprite->setZIndex(history_gauge.foreground, HUD_Z_INDEX);
  api->sprite->setSize(history_gauge.foreground, GRB.width, GRB.height);
  api->sprite->setDrawFunction(history_gauge.foreground, &draw_gauge_bar);
  api->sprite->setUpdateFunction(
    history_gauge.foreground, 
    &noop_sprite_update
  );
  api->sprite->setVisible(history_gauge.foreground, 0);
  api->sprite->addSprite(history_gauge.foreground);
}

static void gauge_crank_update(void* gauge, va_list args) {
  PlaydateAPI* api = get_api();
  int time_diff = va_arg(args, int);
  bool was_showing = 
    history_gauge.ticks_remaining < history_gauge.potential_size;

  uint16_t past_remaining = history_gauge.ticks_remaining;
  uint16_t past_size = history_gauge.potential_size;
  int past_delta = past_size - past_remaining;

  history_gauge.ticks_remaining = min(
    max(past_remaining + time_diff, 0), 
    HISTORY_SIZE
  );
  history_gauge.potential_size = max(history_gauge.ticks_remaining, past_size);

  int delta = 
    history_gauge.potential_size - history_gauge.ticks_remaining;
  if (delta) {
    if (!was_showing) {
      if (history_gauge.sprite_animation_frame < 0) {
        history_gauge.sprite_animation_frame = 0;
      }
      history_gauge.sprite_animation_increment = 1;
    }

    // If history has been fully consumed vibrate as visual indication,
    // relying on the sprite's update function to restory primary position
    // if crank did not happen on a frame.
    if (!history_gauge.ticks_remaining) { 
      history_gauge.vibration = LEAVING;
    }
  } else {
    if (was_showing) {
      if (history_gauge.sprite_animation_frame < 0) {
        history_gauge.sprite_animation_frame = CRANK_ANIMATION_DURATION - 1;
      }
      history_gauge.sprite_animation_increment = -1;
    }
  }

  if (past_remaining != history_gauge.ticks_remaining) {
    api->sprite->markDirty(history_gauge.foreground);
    if (past_size != history_gauge.potential_size) {
      api->sprite->markDirty(history_gauge.unused_capacity);
    }
  }
}

void history_gauge_connect(void) {
  PlaydateAPI* api = get_api();

  if (history_gauge.crank_id != INVALID_GID) {
    api->system->error("History gauge already connected to crank");
  }

  if (
    !history_gauge.background || 
    !history_gauge.unused_capacity || 
    !history_gauge.foreground
  ) {
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
  api->sprite->setVisible(history_gauge.unused_capacity, 0);
  api->sprite->setVisible(history_gauge.foreground, 0);
}
