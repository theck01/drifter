
#include <string.h>

#include "C/api.h"
#include "C/macro.h"
#include "C/utils/geometry.h"

#include "movement-controller.h"

// 1/sqrt(2), used to make sure diagonal movement is as fast as straight line
// movement
const float DIAGONAL_MOVE_FRACTION = 0.70710678118f;

struct movement_controller_struct {
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

  movement_controller* mc = (movement_controller*) self;
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
    mc->direction = active_direction;
    mc->actively_moving = true;
  } else {
    mc->actively_moving = false;
  }

  return NULL;
}

movement_controller* movement_controller_create(
  movement_config* config,
  closure* move, 
  controls* dpad
) {
  movement_controller* mc = malloc(sizeof(movement_controller));
  if (!mc) {
    get_api()->system->error(
      "Could not allocate memory for movement controller"
    );
  }
  
  memcpy(&mc->config, config, sizeof(movement_config));
  mc->move = move;
  mc->c = dpad;
  mc->controls_id = controls_add_listener_for_button_group(
    dpad, 
    closure_create(mc, dpad_listener),
    DPAD
  );

  mc->direction = NONE;
  mc->speed = 0;
  mc->actively_moving = false;

  return mc;
}

int movement_controller_get_speed(movement_controller* mc) {
  return mc->speed;
}

direction_e movement_controller_get_direction(movement_controller* mc) {
  return mc->direction;
}

void movement_controller_step(movement_controller* mc) {
  int old_speed = mc->speed;
  // Update speed based on recent directional inputs
  if (mc->actively_moving) {
    mc->speed = min(
      mc->speed + mc->config.speed_increment_px, 
      mc->config.max_speed_px
    );
  } else {
    mc->speed = max(mc->speed - mc->config.speed_decrement_px, 0);
  }

  // If stopping, send one final move with a 0,0 offset to notify mover that
  // actions have ended (for now)
  if (mc->speed == 0 && old_speed > 0) {
    closure_call(mc->move, 0, 0);
  }

  // Update movers position based on speed
  if (mc->speed > 0) {
    if (mc->direction == NONE) {
      get_api()->system->error(
        "Movement controller has %d speed but no direction", 
        mc->speed
      );
    }

    point offset = { .x = 0, .y = 0 };
    if (mc->direction & U) {
      offset.y -= 1;
    }
    if (mc->direction & D) {
      offset.y += 1;
    }
    if (mc->direction & L) {
      offset.x -= 1;
    }
    if (mc->direction & R) {
      offset.x += 1;
    }

    float move_fraction = offset.x && offset.y ? DIAGONAL_MOVE_FRACTION : 1.f;
    offset.x = offset.x * roundf(move_fraction * mc->speed);
    offset.y = offset.y * roundf(move_fraction * mc->speed);
    closure_call(mc->move, offset.x, offset.y);
  }
}

void movement_controller_destroy(movement_controller* mc) {
  controls_remove_listener_for_button_group(
    mc->c, 
    mc->controls_id, 
    DPAD
  );
  closure_destroy(mc->move);
  free(mc);
}
