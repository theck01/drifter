#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/core/closure.h"
#include "C/core/controls/dpad-movement.h"
#include "C/core/graphics/sprite.h"
#include "C/core/graphics/sprite-animator.h"
#include "C/core/utils/memory/memory-recycler.h"
#include "C/core/utils/random.h"
#include "C/core/utils/types.h"
#include "C/core/world/entity.h"
#include "C/core/world/sensor.h"

#include "drifter.h"
#include "dash-recognizer.h"

// TYPES & CONST

static const char* DRIFTER_LABEL = "DRIFTER";
static const uint8_t MAX_SPEED_PX = 5;
static const uint8_t SPEED_INCREMENT_PX = 1;
static const uint8_t DRIFTER_WIDTH_PX = 18;
static const uint8_t DRIFTER_HEIGHT_PX = 36;

typedef enum {
  IDLE = 0,
  WALK = 1
} action_e;

typedef struct drifter_view_model_struct {
  action_e action;
  direction_e nsew_direction;
} drifter_view_model;

// move closure to replace speed + actively moving model

typedef struct drifter_controls_struct {
  controls* owner;
  dash_recognizer* dr;
  gid_t a_btn;
  gid_t b_btn;
  gid_t crank;
} drifter_controls;

struct drifter_struct {
  entity* self;
  LCDSprite* sprite;
  dpad_movement* dm;
  sprite_animator* animator;
  drifter_controls listeners;
};

// MODEL LOGIC

void* drifter_view_model_allocator(void) {
  drifter_view_model* am = malloc(sizeof(drifter_view_model));
  if (!am) {
    get_api()->system->error("Could not allocate memory for drifter model");
  }
  return am;
}

void drifter_view_model_destructor(void* model) {
  free((drifter_view_model*)model);
}

void drifter_view_model_copy(void * dest, const void* source) {
  drifter_view_model* amd = (drifter_view_model*)dest;
  drifter_view_model* ams = (drifter_view_model*)source;
  memcpy(amd, ams, sizeof(drifter_view_model));
}

// ANIMATION LOADING/LOGIC

// Animations stored with index pattern [action][direction-cipher]
// direction-cipher 0=D,1=U,2=R,3=L
static LCDBitmapTable* drifter_animations[][4] = {
  {NULL, NULL, NULL, NULL },
  {NULL, NULL, NULL, NULL }
};

typedef struct drifter_animation_description_struct {
  char* name;
  action_e action;
  direction_e direction;
} drifter_animation_description;
static drifter_animation_description animation_description[] = {
  { .name = "img/drifter/idle-forward.gif", .action = IDLE, .direction = 0 /* D */ },
  { .name = "img/drifter/idle-away.gif", .action = IDLE, .direction = 1 /* U */ },
  { .name = "img/drifter/idle-right.gif", .action = IDLE, .direction = 2 /* R */ },
  { .name = "img/drifter/idle-left.gif", .action = IDLE, .direction = 3 /* L */ },
  { .name = "img/drifter/walk-forward.gif", .action = WALK, .direction = 0 /* D */ },
  { .name = "img/drifter/walk-away.gif", .action = WALK, .direction = 1 /* U */ },
  { .name = "img/drifter/walk-right.gif", .action = WALK, .direction = 2 /* R */ },
  { .name = "img/drifter/walk-left.gif", .action = WALK, .direction = 3 /* L */ }
};
static const uint8_t animation_count = 8;
static bool drifter_animations_loaded = false;

static uint8_t animation_index_for_direction(direction_e dir) {
  if (dir & D) return 0;
  else if (dir & U) return 1;
  else if (dir & R) return 2;
  else if (dir & L) return 3;
  get_api()->system->error("Cannot get animation index for direction %d", dir);
  return 0;
}

static void load_animations_if_needed(void) {
  if (!drifter_animations_loaded) {
		const char* err;
    PlaydateAPI* api = get_api();
    for (uint8_t i=0; i < animation_count; i++) {
      drifter_animation_description description = animation_description[i];
      drifter_animations[description.action][description.direction] =
        api->graphics->loadBitmapTable(description.name, &err);

      if (!drifter_animations[description.action][description.direction]) {
        api->system->error(
          "Could not load drifter animation \"%s\" because of error \"%s\"", 
          description.name, 
          err
        );
      }
    }
    drifter_animations_loaded = true;
  }
}

// DRIFTER LOGIC

void* drifter_move(void* self, va_list args) {
  drifter* d = (drifter*)self;
  drifter_view_model* model = (drifter_view_model*)entity_get_model(d->self);
  int dx = va_arg(args, int);
  int dy = va_arg(args, int);

  direction_e dir = dpad_movement_get_direction(d->dm);
  if (!(dir & model->nsew_direction)) {
    if (dir & D) {
      model->nsew_direction = D;
    } else if (dir & U) {
      model->nsew_direction = U;
    } else if (dir & R) {
      model->nsew_direction = R;
    } else if (dir & L) {
      model->nsew_direction = L;
    } else {
      get_api()->system->error("Cannot change to drifter to NONE direction");
    }
  }

  // If movement has completed, signified with a (0,0) difference, then
  // change the drifters action to IDLE
  if (dx == 0 && dy == 0) {
    model->action = IDLE;
  } else {
    model->action = WALK;

    sensor* s = entity_get_sensor(d->self);
    point p, actual;
    entity_get_position(d->self, &p);
    p.x += dx;
    p.y += dy;
    if (sensor_can_entity_move(s, d->self, p, &actual)) {
      entity_move_to(d->self, p);
    }
  }

  return NULL;
}

void* drifter_dash(void* self, va_list args) {
  drifter* d = (drifter*)self;
  direction_e dir = (direction_e)va_arg(args, int);
  get_api()->system->logToConsole("drifter dash in %x direction", dir);
  return NULL;
}

void* drifter_plan(void* self, va_list args) {
  drifter* d = (drifter*)self;
  dpad_movement_step(d->dm);
  return NULL;
}

void* drifter_apply(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  drifter* d = (drifter*)self;
  drifter_view_model* current = va_arg(args, drifter_view_model*);
  drifter_view_model* prev = va_arg(args, drifter_view_model*);
  int did_move = va_arg(args, int);

  if (did_move) {
    point p;
    entity_get_position(d->self, &p);
    api->sprite->moveTo(d->sprite, p.x, p.y);
  }

  if (
    !prev || 
    current->action != prev->action
  ) {
    sprite_animator_set_animation_and_frame(
      d->animator, 
      drifter_animations[current->action][
       animation_index_for_direction(current->nsew_direction)
      ],
      0 /* starting frame */
    );
  } else if (current->nsew_direction != prev->nsew_direction) {
    sprite_animator_set_animation(
      d->animator, 
      drifter_animations[current->action][
        animation_index_for_direction(current->nsew_direction)
      ]
    );
  }
  return NULL;
}

void* drifter_show(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  drifter* d = (drifter*)self;
  int show = (bool)va_arg(args, int);
  api->sprite->setVisible(d->sprite, show);
  if (show) {
    sprite_animator_resume(d->animator);
  } else {
    sprite_animator_pause(d->animator);
  }
  return NULL;
}

void* drifter_spawn(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  drifter* d = (drifter*)self;
  if (d->sprite) {
    api->system->error("Cannot spawn drifter that already has spawned");
  }

  point p;
  entity_get_position(d->self, &p);

  d->sprite = create_entity_sprite();
  api->sprite->moveTo(d->sprite, p.x, p.y);
  api->sprite->addSprite(d->sprite);
  api->sprite->setVisible(d->sprite, false);

  drifter_view_model* model = (drifter_view_model*)entity_get_model(d->self);
  d->animator = sprite_animator_create(
    d->sprite,
    drifter_animations[model->action][
      animation_index_for_direction(model->nsew_direction)
    ],
    12 /* fps */, 
    0 /* starting_frame */
  );
  sprite_animator_start(d->animator);
  
  return NULL;
}

void* drifter_despawn(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  drifter* d = (drifter*)self;
  if (!d->sprite) {
    api->system->error("Cannot despawn drifter that is already despawned");
  }

  api->sprite->removeSprite(d->sprite);
  api->sprite->freeSprite(d->sprite);
  d->sprite = NULL;
  sprite_animator_destroy(d->animator);
  d->animator = NULL;

  return NULL;
}

void* drifter_button_press(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  input_event* button_events = va_arg(args, input_event*);
  int i = 0;
  while (!input_event_is_nil(button_events[i])) {
    api->system->logToConsole("Button event: %d", button_events[i]);
    i++;
  }
  return NULL;
}

void* drifter_crank(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  crank_event* ce = va_arg(args, crank_event*);
  api->system->logToConsole(
    "Crank event: { tick: %d, diff: %d }", 
    ce->tick, 
    ce->diff
  );
  return NULL;
}

drifter* drifter_create(world* w, controls* c, point* p) {
  load_animations_if_needed();
  PlaydateAPI* api = get_api();
  drifter* d = malloc(sizeof(drifter));
  if (!d) {
    get_api()->system->error("Could not allocate memory for drifter");
  }

  drifter_view_model initial_extended = {
    .action = IDLE,
    .nsew_direction = D
  };

  point size = { 
    .x = DRIFTER_WIDTH_PX, 
    .y = DRIFTER_HEIGHT_PX 
  };
  entity_behavior behavior = {
    .spawn = closure_create(d, drifter_spawn),
    .show = closure_create(d, drifter_show),
    .apply = closure_create(d, drifter_apply),
    .despawn = closure_create(d, drifter_despawn),
    .plan = closure_create(d, drifter_plan)
  };
  d->self = entity_create(
    DRIFTER_LABEL,
    p,
    &size,
    &initial_extended,
    &behavior,
    drifter_view_model_allocator,
    drifter_view_model_destructor,
    drifter_view_model_copy
  );

  movement_config config = {
    .max_speed_px = 5,
    .speed_increment_px = 1,
    .speed_decrement_px = 2,
  };
  d->dm = dpad_movement_create(
    &config,
    closure_create(d, drifter_move),
    c
  );

  d->sprite = NULL;
  d->animator = NULL;

  closure* btn_listener = closure_create(d, drifter_button_press);
  // Retain btn closure twice, so that it can withstand being destroyed by 2 
  // removals needed to fully disconnect the listener from all events
  closure_retain(btn_listener);

  d->listeners.owner = c;

  d->listeners.dr = dash_recognizer_create(c, closure_create(d, drifter_dash));

  d->listeners.a_btn = 
    controls_add_listener_for_button_group(c, btn_listener, A_BTN);
  d->listeners.b_btn = 
    controls_add_listener_for_button_group(c, btn_listener, B_BTN);
  d->listeners.crank = 
    controls_add_crank_listener(c, closure_create(d, drifter_crank));

  world_add_entity(w, d->self);

  return d;
}

entity* drifter_get_entity(drifter* d) {
  return d->self;
}


void drifter_destroy(drifter* d) {
  entity_destroy(d->self);  
  dpad_movement_destroy(d->dm);

  dash_recognizer_destroy(d->listeners.dr);

  controls_remove_listener_for_button_group(
    d->listeners.owner, 
    d->listeners.a_btn,
    A_BTN
  );
  controls_remove_listener_for_button_group(
    d->listeners.owner, 
    d->listeners.b_btn,
    B_BTN
  );
  controls_remove_crank_listener(d->listeners.owner, d->listeners.crank);

  free(d);
}
