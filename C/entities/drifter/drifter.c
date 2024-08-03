#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "C/api.h"
#include "C/const.h"
#include "C/macro.h"
#include "C/core/animation/sprite-animator.h"
#include "C/core/controls/dpad-movement.h"
#include "C/core/world/entity.h"
#include "C/core/world/sensor.h"
#include "C/utils/closure.h"
#include "C/utils/memory-recycler.h"
#include "C/utils/random.h"
#include "C/utils/sprite.h"
#include "C/utils/types.h"

#include "drifter.h"


// TYPES & CONST

static char* DRIFTER_LABEL = "DRIFTER";
static const uint8_t MAX_SPEED_PX = 5;
static const uint8_t SPEED_INCREMENT_PX = 1;
static const uint8_t DRIFTER_WIDTH_PX = 18;
static const uint8_t DRIFTER_HEIGHT_PX = 36;

typedef enum {
  IDLE = 0,
  WALK = 1
} action_e;

typedef struct drifter_model_struct {
  action_e action;
  direction_e direction;
} drifter_model;

// move closure to replace speed + actively moving model

typedef struct drifter_listeners_struct {
  controls* owner;
  gid_t a_btn;
  gid_t b_btn;
  gid_t crank;
} drifter_listeners;

struct drifter_struct {
  entity* self;
  LCDSprite* sprite;
  dpad_movement* dm;
  sprite_animator* animator;
  drifter_listeners listeners;
};

// MODEL LOGIC

void* drifter_model_allocator(void) {
  drifter_model* am = malloc(sizeof(drifter_model));
  if (!am) {
    get_api()->system->error("Could not allocate memory for drifter model");
  }
  return am;
}

void drifter_model_destructor(void* model) {
  free((drifter_model*)model);
}

void drifter_model_copy(void * dest, void* source) {
  drifter_model* amd = (drifter_model*)dest;
  drifter_model* ams = (drifter_model*)source;
  memcpy(amd, ams, sizeof(drifter_model));
}

// ANIMATION LOADING

// Animations stored with index pattern [action]
static LCDBitmapTable* drifter_animations[] = {
  NULL,
  NULL,
};

typedef struct drifter_animation_description_struct {
  char* name;
  action_e action;
} drifter_animation_description;
static drifter_animation_description animation_description[] = {
  { .name = "img/drifter/idle.gif", .action = IDLE },
  { .name = "img/drifter/walk.gif", .action = WALK },
};
static const uint8_t animation_count = 2;
static bool drifter_animations_loaded = false;

static void load_animations_if_needed(void) {
  if (!drifter_animations_loaded) {
		const char* err;
    PlaydateAPI* api = get_api();
    for (uint8_t i=0; i < animation_count; i++) {
      drifter_animation_description description = animation_description[i];
      drifter_animations[description.action] =
        api->graphics->loadBitmapTable(description.name, &err);

      if (!drifter_animations[description.action]) {
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
  drifter_model* model = (drifter_model*)entity_get_model(d->self);
  int dx = va_arg(args, int);
  int dy = va_arg(args, int);

  model->direction = dpad_movement_get_direction(d->dm);

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

void* drifter_plan(void* self, va_list args) {
  drifter* d = (drifter*)self;
  dpad_movement_step(d->dm);
  return NULL;
}

void* drifter_apply(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  drifter* d = (drifter*)self;
  drifter_model* current = va_arg(args, drifter_model*);
  drifter_model* prev = va_arg(args, drifter_model*);
  int did_move = va_arg(args, int);

  if (did_move) {
    point p;
    entity_get_position(d->self, &p);
    api->sprite->moveTo(d->sprite, p.x, p.y);
  }
  if (!prev || current->action != prev->action) {
    sprite_animator_set_animation_and_frame(
      d->animator, 
      drifter_animations[current->action],
      0 /* starting frame */
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

  d->sprite = create_draw_only_sprite();
  api->sprite->moveTo(d->sprite, p.x, p.y);
  api->sprite->setZIndex(d->sprite, ACTOR_Z_INDEX);
  api->sprite->addSprite(d->sprite);
  api->sprite->setVisible(d->sprite, false);

  drifter_model* model = (drifter_model*)entity_get_model(d->self);
  d->animator = sprite_animator_create(
    d->sprite,
    drifter_animations[model->action],
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

  drifter_model initial_extended = {
    .action = IDLE,
    .direction = NONE,
  };
  int_rect bounds = { 
    .x = p->x, 
    .y = p->y, 
    .width = DRIFTER_WIDTH_PX, 
    .height = DRIFTER_HEIGHT_PX 
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
    &bounds,
    &initial_extended,
    &behavior,
    drifter_model_allocator,
    drifter_model_destructor,
    drifter_model_copy
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

  world_add_entity(w, d->self);


  closure* btn_listener = closure_create(d, drifter_button_press);
  // Retain btn closure twice, so that it can withstand being destroyed by 2 
  // removals needed to fully disconnect the listener from all events
  closure_retain(btn_listener);

  d->listeners.owner = c;
  d->listeners.a_btn = 
    controls_add_listener_for_button_group(c, btn_listener, A_BTN);
  d->listeners.b_btn = 
    controls_add_listener_for_button_group(c, btn_listener, B_BTN);
  d->listeners.crank = 
    controls_add_crank_listener(c, closure_create(d, drifter_crank));

  return d;
}

entity* drifter_get_entity(drifter* d) {
  return d->self;
}


void drifter_destroy(drifter* d) {
  PlaydateAPI* api = get_api();
  entity_destroy(d->self);  
  dpad_movement_destroy(d->dm);
  sprite_animator_destroy(d->animator);
  api->sprite->removeSprite(d->sprite);
  api->sprite->freeSprite(d->sprite);

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
