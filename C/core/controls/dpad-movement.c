
#include <string.h>

#include "C/api.h"
#include "C/macro.h"
#include "C/utils/geometry.h"

#include "dpad-movement.h"

// 1/sqrt(2), used to make sure diagonal movement is as fast as straight line
// movement
const float DIAGONAL_MOVE_FRACTION = 0.70710678118f;

struct dpad_movement_struct {
  movement_config config;
  closure* move;
  controls* c;
  gid_t controls_id;
  direction_e direction;
  int speed;
  bool actively_moving;
};

static void* dpad_listener(void* self, va_list args) {
  PlaydateAPI* api = get_api();

  dpad_movement* dm = (dpad_movement*) self;
  input_event* dpad_events = va_arg(args, input_event*);

  direction_e active_direction = NONE;
  int i = 0;
  while (!input_event_is_nil(dpad_events[i])) {
    input_button_e btn = input_event_button(dpad_events[i]);
    direction_e btn_direction = input_button_to_direction(btn);
    input_action_e action = input_event_action(dpad_events[i]);
    if (action == HELD || action == PRESS) {
      active_direction |= btn_direction;
    }
    i++;
  }

  if (active_direction != NONE) {
    dm->direction = active_direction;
    dm->actively_moving = true;
  } else {
    dm->actively_moving = false;
  }

  return NULL;
}

dpad_movement* dpad_movement_create(
  movement_config* config,
  closure* move, 
  controls* dpad
) {
  dpad_movement* dm = malloc(sizeof(dpad_movement));
  if (!dm) {
    get_api()->system->error(
      "Could not allocate memory for movement controller"
    );
  }
  
  memcpy(&dm->config, config, sizeof(movement_config));
  dm->move = move;
  dm->c = dpad;
  dm->controls_id = controls_add_listener_for_button_group(
    dpad, 
    closure_create(dm, dpad_listener),
    DPAD
  );

  dm->direction = NONE;
  dm->speed = 0;
  dm->actively_moving = false;

  return dm;
}

int dpad_movement_get_speed(dpad_movement* dm) {
  return dm->speed;
}

direction_e dpad_movement_get_direction(dpad_movement* dm) {
  return dm->direction;
}

void dpad_movement_step(dpad_movement* dm) {
  int old_speed = dm->speed;
  // Update speed based on recent directional inputs
  if (dm->actively_moving) {
    dm->speed = min(
      dm->speed + dm->config.speed_increment_px, 
      dm->config.max_speed_px
    );
  } else {
    dm->speed = max(dm->speed - dm->config.speed_decrement_px, 0);
  }

  // If stopping, send one final move with a 0,0 offset to notify mover that
  // actions have ended (for now)
  if (dm->speed == 0 && old_speed > 0) {
    closure_call(dm->move, 0, 0);
  }

  // Update movers position based on speed
  if (dm->speed > 0) {
    if (dm->direction == NONE) {
      get_api()->system->error(
        "Movement controller has %d speed but no direction", 
        dm->speed
      );
    }

    point offset = { .x = 0, .y = 0 };
    if (dm->direction & U) {
      offset.y -= 1;
    }
    if (dm->direction & D) {
      offset.y += 1;
    }
    if (dm->direction & L) {
      offset.x -= 1;
    }
    if (dm->direction & R) {
      offset.x += 1;
    }

    float move_fraction = offset.x && offset.y ? DIAGONAL_MOVE_FRACTION : 1.f;
    offset.x = offset.x * roundf(move_fraction * dm->speed);
    offset.y = offset.y * roundf(move_fraction * dm->speed);
    closure_call(dm->move, offset.x, offset.y);
  }
}

void dpad_movement_destroy(dpad_movement* dm) {
  controls_remove_listener_for_button_group(
    dm->c, 
    dm->controls_id, 
    DPAD
  );
  closure_destroy(dm->move);
  free(dm);
}
