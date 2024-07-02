
#include <stdbool.h>
#include <string.h>

#include "C/api.h"
#include "C/core/sprite-animator.h"
#include "C/core/entity.h"
#include "C/core/sensor.h"
#include "C/utils/closure.h"
#include "C/utils/memory-recycler.h"
#include "C/utils/random.h"
#include "C/utils/sprite.h"
#include "C/utils/types.h"

#include "ant.h"


// TYPES & CONST

static char* ANT_LABEL = "ANT";
// MUST BE POWER OF 2
static const uint8_t MAX_SPEED_PX = 8;
static const uint8_t ANT_WIDTH_PX = 29;
static const uint8_t ANT_HEIGHT_PX = 25;

typedef enum {
  LEFT = 0,
  RIGHT = 1
} orientation_e;

typedef enum {
  IDLE = 0,
  WALK = 1
} action_e;

typedef struct ant_model_struct {
  action_e action;
  orientation_e orientation;
  uint8_t speed;
  uint8_t ticks_to_next_decision;
} ant_model;

struct ant_struct {
  entity* self;
  LCDSprite* sprite;
  sprite_animator* animator;
  gid_t id;
};


// MODEL LOGIC

void* ant_model_allocator(void) {
  ant_model* am = malloc(sizeof(ant_model));
  if (!am) {
    get_api()->system->error("Could not allocate memory for ant model");
  }
  return am;
}

void ant_model_destructor(void* model) {
  free((ant_model*)model);
}

void ant_model_copy(void * dest, void* source) {
  ant_model* amd = (ant_model*)dest;
  ant_model* ams = (ant_model*)source;
  memcpy(amd, ams, sizeof(ant_model));
}

// ANIMATION LOADING

// Animations stored with index pattern [action][orientation]
static LCDBitmapTable* ant_animations[][2] = {
  {NULL, NULL},
  {NULL, NULL}
};

typedef struct ant_animation_description_struct {
  char* name;
  action_e action;
  orientation_e orientation;
} ant_animation_description;
static ant_animation_description animation_description[] = {
  { .name = "img/ant-idle-left.gif", .action = IDLE, .orientation = LEFT },
  { .name = "img/ant-idle-right.gif", .action = IDLE, .orientation = RIGHT },
  { .name = "img/ant-walk-left.gif", .action = WALK, .orientation = LEFT },
  { .name = "img/ant-walk-right.gif", .action = WALK, .orientation = RIGHT },
};
const uint8_t animation_count = 4;
bool ant_animations_loaded = false;

static void load_animations_if_needed(void) {
  if (!ant_animations_loaded) {
		const char* err;
    PlaydateAPI* api = get_api();
    for (uint8_t i=0; i < animation_count; i++) {
      ant_animation_description description = animation_description[i];
      ant_animations[description.action][description.orientation] =
        api->graphics->loadBitmapTable(description.name, &err);

      if (!ant_animations[description.action][description.orientation]) {
        api->system->error(
          "Could not load ant animation \"%s\" because of error \"%s\"", 
          description.name, 
          err
        );
      }
    }
    ant_animations_loaded = true;
  }
}


// ANT LOGIC

void* ant_plan(void* self, va_list args) {
  ant* a = (ant*)self;
  ant_model* model = va_arg(args, ant_model*);
  sensor* viewpoint = va_arg(args, sensor*);

  if (model->action == WALK) {
    int16_t velocity = model->speed * 
      (model->orientation == RIGHT ? 1 : -1);
    point destination, actual;
    entity_get_position(a->self, &destination);
    destination.x += velocity;
    if (sensor_can_entity_move(viewpoint, a->self, destination, &actual)) {
      entity_move_to(a->self, destination);
    } else {
      model->orientation = 
        model->orientation == LEFT ? RIGHT : LEFT;
    }

    if (model->ticks_to_next_decision < 4) {
      model->speed >>= 1;
    } else if (model->speed < MAX_SPEED_PX) {
      model->speed <<= 1;
    }
  }

  if (model->ticks_to_next_decision == 0) {
    model->action = randomf() > 0.5f ? WALK : IDLE;
    if (model->action == WALK) {
      model->speed = 1;
    }
    model->orientation = randomf() > 0.5f ? LEFT : RIGHT;
    model->ticks_to_next_decision = random_uint(15,60);
  }
  model->ticks_to_next_decision--;
  return NULL;
}

void* ant_apply(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  ant* a = (ant*)self;
  ant_model* current = va_arg(args, ant_model*);
  ant_model* prev = va_arg(args, ant_model*);
  int did_move = va_arg(args, int);

  if (did_move) {
    point p;
    entity_get_position(a->self, &p);
    api->sprite->moveTo(a->sprite, p.x, p.y);
  }
  if (!prev || current->action != prev->action) {
    sprite_animator_set_animation_and_frame(
      a->animator, 
      ant_animations[current->action][current->orientation],
      0 /* starting frame */
    );
  } else if (!prev || current->orientation != prev->orientation) {
    sprite_animator_set_animation(
      a->animator, 
      ant_animations[current->action][current->orientation]
    );
  }
  return NULL;
}

void* ant_show(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  ant* a = (ant*)self;
  int show = (bool)va_arg(args, int);
  api->sprite->setVisible(a->sprite, show);
  if (show) {
    sprite_animator_resume(a->animator);
  } else {
    sprite_animator_pause(a->animator);
  }
  return NULL;
}

void* ant_spawn(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  ant* a = (ant*)self;
  if (a->sprite) {
    api->system->error("Cannot spawn ant that already has spawned");
  }

  ant_model* model = va_arg(args, ant_model*);

  point p;
  entity_get_position(a->self, &p);

  a->sprite = create_draw_only_sprite();
  api->sprite->moveTo(a->sprite, p.x, p.y);
  api->sprite->setZIndex(a->sprite, ACTOR_Z_INDEX);
  api->sprite->addSprite(a->sprite);
  api->sprite->setVisible(a->sprite, false);

  a->animator = sprite_animator_create(
    a->sprite,
    ant_animations[model->action][model->orientation], 
    12 /* fps */, 
    0 /* starting_frame */
  );
  sprite_animator_start(a->animator);
  sprite_animator_pause(a->animator);
  
  return NULL;
}

void* ant_despawn(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  ant* a = (ant*)self;
  if (!a->sprite) {
    api->system->error("Cannot despawn ant that is already despawned");
  }

  api->sprite->freeSprite(a->sprite);
  a->sprite = NULL;
  sprite_animator_destroy(a->animator);
  a->animator = NULL;

  return NULL;
}

ant* ant_create(world* w, int x, int y) {
  load_animations_if_needed();
  PlaydateAPI* api = get_api();
  ant* a = malloc(sizeof(ant));
  if (!a) {
    get_api()->system->error("Could not allocate memory for ant");
  }

  a->id = getNextGID();

  ant_model initial_extended = {
    .action = IDLE,
    .orientation = RIGHT,
    .speed = 0,
    .ticks_to_next_decision = random_uint(15, 60)
  };
  int_rect bounds = { 
    .x = x, 
    .y = y, 
    .width = ANT_WIDTH_PX, 
    .height = ANT_HEIGHT_PX 
  };
  entity_behavior behavior = {
    .spawn = closure_create(a, ant_spawn),
    .show = closure_create(a, ant_show),
    .apply = closure_create(a, ant_apply),
    .despawn = closure_create(a, ant_despawn),
    .plan = closure_create(a, ant_plan)
  };
  a->self = entity_create(
    ANT_LABEL,
    &bounds,
    &initial_extended,
    &behavior,
    ant_model_allocator,
    ant_model_destructor,
    ant_model_copy
  );

  a->sprite = NULL;
  a->animator = NULL;

  world_add_entity(w, a->self);

  return a;
}

void ant_destroy(ant* a) {
  PlaydateAPI* api = get_api();
  entity_destroy(a->self);  
  sprite_animator_destroy(a->animator);
  api->sprite->removeSprite(a->sprite);
  api->sprite->freeSprite(a->sprite);
  free(a);
}
