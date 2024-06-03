
#include <stdbool.h>

#include "C/api.h"
#include "C/core/sprite-animator.h"
#include "C/core/entity.h"
#include "C/utils/closure.h"
#include "C/utils/functions.h"
#include "C/utils/memory-recycler.h"
#include "C/utils/random.h"
#include "C/utils/types.h"

#include "ant.h"


// TYPES & CONST

static char* ANT_LABEL = "ANT";
// MUST BE POWER OF 2
static const uint8_t MAX_SPEED_PX = 8;

typedef enum {
  LEFT = 0,
  RIGHT = 1
} orientation_e;

typedef enum {
  IDLE = 0,
  WALK = 1
} action_e;

typedef struct ant_model_struct {
  float x;
  float y;
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

void ant_model_copy(void* source, void* dest) {
  ant_model* ams = (ant_model*)source;
  ant_model* amd = (ant_model*)dest;
  amd->x = ams->x;
  amd->y = ams->y;
  amd->action = ams->action;
  amd->orientation = ams->orientation;
  amd->speed = ams->speed;
  amd->ticks_to_next_decision = ams->ticks_to_next_decision;
}

void ant_model_print(ant_model* am) {
  get_api()->system->logToConsole(
    "{ x: %f, y: %f, action: %d, orientation: %d, speed: %d, ticks_to_next_decision: %d }", 
    am->x, 
    am->y, 
    am->action, 
    am->orientation, 
    am->speed, 
    am->ticks_to_next_decision
  );
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

void ant_init(void* initial_model, va_list args) {
  void* model_to_init = va_arg(args, void*);
  ant_model_copy(initial_model, model_to_init);
}

void ant_plan(void* self, va_list args) {
  ant* a = (ant*)self;
  ant_model* to_update = va_arg(args, ant_model*);
  ant_model* current_state = va_arg(args, ant_model*);

  if (current_state->action == WALK) {
    int16_t velocity = current_state->speed * 
      (current_state->orientation == RIGHT ? 1 : -1);
    to_update->x += velocity;

    if (current_state->ticks_to_next_decision < 4) {
      to_update->speed >>= 1;
    } else if (current_state->speed < MAX_SPEED_PX) {
      to_update->speed <<= 1;
    }
  }

  if (current_state->ticks_to_next_decision == 0) {
    to_update->action = randomf() > 0.5f ? WALK : IDLE;
    if (to_update->action == WALK) {
      to_update->speed = 1;
    }
    to_update->orientation = randomf() > 0.5f ? LEFT : RIGHT;
    to_update->ticks_to_next_decision = random_uint(15,60);
  }
  to_update->ticks_to_next_decision--;
}

void ant_apply(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  ant* a = (ant*)self;
  ant_model* current_model = va_arg(args, ant_model*);
  ant_model* prev_model = va_arg(args, ant_model*);

  if (
    !prev_model || 
    current_model->x != prev_model->x || 
    current_model->y != prev_model->y
  ) {
    api->sprite->moveTo(a->sprite, current_model->x, current_model->y);
  }
  if (!prev_model || current_model->action != prev_model->action) {
    sprite_animator_set_animation_and_frame(
      a->animator, 
      ant_animations[current_model->action][current_model->orientation],
      0 /* starting frame */
    );
  } else if (
    !prev_model || 
    current_model->orientation != prev_model->orientation
  ) {
    sprite_animator_set_animation(
      a->animator, 
      ant_animations[current_model->action][current_model->orientation]
    );
  }
}

ant* ant_create(float x, float y) {
  load_animations_if_needed();
  PlaydateAPI* api = get_api();
  ant* a = malloc(sizeof(ant));
  if (!a) {
    get_api()->system->error("Could not allocate memory for ant");
  }

  ant_model initial_model = {
    .x = x,
    .y = y,
    .action = IDLE,
    .orientation = RIGHT,
    .speed = 0,
    .ticks_to_next_decision = random_uint(15, 60)
  };

  a->id = getNextGID();

  a->self = entity_create(
    ANT_LABEL,
    ant_model_allocator,
    ant_model_destructor,
    ant_model_copy
  );

  a->sprite = api->sprite->newSprite();
  api->sprite->setUpdateFunction(a->sprite, &noop_sprite_update);
  api->sprite->setZIndex(a->sprite, ACTOR_Z_INDEX);
  api->sprite->moveTo(a->sprite, x, y);
  api->sprite->addSprite(a->sprite);

  a->animator = sprite_animator_create(
    a->sprite,
    ant_animations[initial_model.action][initial_model.orientation], 
    12 /* fps */, 
    0 /* starting_frame */
  );
  sprite_animator_start(a->animator);

  entity_start_active(
    a->self,
    closure_create(&initial_model, ant_init),
    closure_create(a, ant_apply),
    closure_create(a, ant_plan)
  );

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
