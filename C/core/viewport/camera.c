
#include <string.h>
#include <stdbool.h>

#include "C/api.h"
#include "C/macro.h"
#include "C/core/controls/dpad-movement.h"
#include "C/core/clock.h"
#include "C/utils/closure.h"
#include "C/utils/types.h"


#include "viewport.h"

#include "camera.h"

static const int MAX_MOVE_SPEED_PX = 8;
static const int MOVE_SPEED_INCREMENT_PX = 1;
static const int MOVE_SPEED_DECREMENT_PX = 2;
static const int_rect TRACKING_AREA_WITHIN_VIEWPORT = {
  .x = 120,
  .y = 80,
  .width = 160,
  .height = 80
};

struct camera_struct {
  world* w;
  point origin;
  gid_t clock_id;
  entity* tracked;
  dpad_movement* dm;
  camera_mode_e mode;
};

void camera_keep_entity_within_bounds(camera* c) {
  if (!c->tracked) {
    get_api()->system->error("Cannot frame camera around non-existant entity");
  }

  point offset;
  viewport_get_offset(&offset);
  int_rect tracking_bounds = {
    .x = offset.x + TRACKING_AREA_WITHIN_VIEWPORT.x, 
    .y = offset.y + TRACKING_AREA_WITHIN_VIEWPORT.y, 
    .width = TRACKING_AREA_WITHIN_VIEWPORT.width,
    .height = TRACKING_AREA_WITHIN_VIEWPORT.height,
  };
  LCDRect tracking_area;
  int_rect_to_lcd(&tracking_bounds, &tracking_area);

  int_rect entity_bounds;
  entity_get_bounds(c->tracked, &entity_bounds);
  LCDRect entity_area;
  int_rect_to_lcd(&entity_bounds, &entity_area);

  point shift_amount = { .x = 0, .y = 0 };
  if (entity_area.left < tracking_area.left) {
    shift_amount.x = entity_area.left - tracking_area.left;
  }
  if (entity_area.right > tracking_area.right) {
    shift_amount.x = entity_area.right - tracking_area.right;
  }
  if (entity_area.top < tracking_area.top) {
    shift_amount.y = entity_area.top - tracking_area.top;
  }
  if (entity_area.bottom > tracking_area.bottom) {
    shift_amount.y = entity_area.bottom - tracking_area.bottom;
  }

  if (shift_amount.x != 0 || shift_amount.y != 0) {
    viewport_set_offset(offset.x + shift_amount.x, offset.y + shift_amount.y);
  }
}


void* camera_time_advance(void* context, va_list args) {
  camera* c = (camera*)context;
  int current_time = va_arg(args, int);
  clock_mask_e clock_mask = (clock_mask_e)va_arg(args, int);

  if (c->mode == MANUAL_MOVEMENT) {
    dpad_movement_step(c->dm);
  }

  if (c->tracked) {
    if (clock_mask & END) {
      camera_keep_entity_within_bounds(c);
    }
  }

  return NULL;
}

camera* camera_create(
  world* w, 
  point origin
) {
  camera* c = malloc(sizeof(camera));
  if (!c) {
    get_api()->system->error("Could not allocate memory for camera");
  }

  c->w = w;
  memcpy(&c->origin, &origin, sizeof(point));
  c->clock_id = clock_add_listener(closure_create(c, camera_time_advance));
  c->tracked = NULL;
  c->dm = NULL;
  c->mode = FIXED;

  viewport_set_offset(origin.x, origin.y);

  return c;
}

void camera_teardown_existing_mode(camera* c) {
  if (c->mode == MANUAL_MOVEMENT) {
    dpad_movement_destroy(c->dm);
    c->dm = NULL;
  } else if (c->mode == TRACKING) {
    c->tracked = NULL;
  }
}

void camera_track(camera* c, entity* e) {
  camera_teardown_existing_mode(c);
  c->tracked = e;
  c->mode = TRACKING;
}

void* camera_move(void* context, va_list args) {
  camera* c = context;

  int dx = va_arg(args, int);
  int dy = va_arg(args, int);

  point offset;
  viewport_get_offset(&offset);
  viewport_set_offset(offset.x + dx, offset.y + dy);

  return NULL;
}

void camera_control_with_dpad(camera* c, controls* dpad) {
  camera_teardown_existing_mode(c);

  movement_config config = {
    .max_speed_px = 6,
    .speed_increment_px = 1,
    .speed_decrement_px = 2,
  };
  c->dm = dpad_movement_create(
    &config,
    closure_create(c, camera_move),
    dpad
  );
  c->mode = MANUAL_MOVEMENT;
}

void camera_fix(camera* c) {
  camera_teardown_existing_mode(c);
  c->mode = FIXED;
}

camera_mode_e camera_get_mode(camera* c) {
  return c->mode;
}

void camera_destroy(camera* c) {
  camera_teardown_existing_mode(c);
  clock_remove_listener(c->clock_id);
}
