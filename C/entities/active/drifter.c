
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

#include "drifter.h"


// TYPES & CONST

static char* DRIFTER_LABEL = "DRIFTER";
// MUST BE POWER OF 2
static const uint8_t MAX_SPEED_PX = 8;
static const uint8_t DRIFTER_WIDTH_PX = 18;
static const uint8_t DRIFTER_HEIGHT_PX = 36;

typedef enum {
  IDLE = 0,
  WALK = 1,
  MEDIUM = 2,
  LARGE = 3,
} action_e;

typedef struct drifter_model_struct {
  action_e action;
} drifter_model;

struct drifter_struct {
  entity* self;
  LCDSprite* sprite;
  sprite_animator* animator;
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
  NULL,
  NULL
};

typedef struct drifter_animation_description_struct {
  char* name;
  action_e action;
} drifter_animation_description;
static drifter_animation_description animation_description[] = {
  { .name = "img/drifter/tiny-idle.gif", .action = IDLE },
  { .name = "img/drifter/tiny-walk.gif", .action = WALK },
  { .name = "img/drifter/idle.gif", .action = MEDIUM },
  { .name = "img/drifter/large.gif", .action = LARGE }
};
static const uint8_t animation_count = 4;
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

void* drifter_plan(void* self, va_list args) {
  drifter* a = (drifter*)self;
  drifter_model* model = va_arg(args, drifter_model*);
  sensor* viewpoint = va_arg(args, sensor*);
  return NULL;
}

void* drifter_apply(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  drifter* a = (drifter*)self;
  drifter_model* current = va_arg(args, drifter_model*);
  drifter_model* prev = va_arg(args, drifter_model*);
  int did_move = va_arg(args, int);

  if (did_move) {
    point p;
    entity_get_position(a->self, &p);
    api->sprite->moveTo(a->sprite, p.x, p.y);
  }
  if (!prev || current->action != prev->action) {
    sprite_animator_set_animation_and_frame(
      a->animator, 
      drifter_animations[current->action],
      0 /* starting frame */
    );
  }
  return NULL;
}

void* drifter_show(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  drifter* a = (drifter*)self;
  int show = (bool)va_arg(args, int);
  api->sprite->setVisible(a->sprite, show);
  if (show) {
    sprite_animator_resume(a->animator);
  } else {
    sprite_animator_pause(a->animator);
  }
  return NULL;
}

void* drifter_spawn(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  drifter* a = (drifter*)self;
  if (a->sprite) {
    api->system->error("Cannot spawn drifter that already has spawned");
  }

  drifter_model* model = va_arg(args, drifter_model*);

  point p;
  entity_get_position(a->self, &p);

  a->sprite = create_draw_only_sprite();
  api->sprite->moveTo(a->sprite, p.x, p.y);
  api->sprite->setZIndex(a->sprite, ACTOR_Z_INDEX);
  api->sprite->addSprite(a->sprite);
  api->sprite->setVisible(a->sprite, false);

  a->animator = sprite_animator_create(
    a->sprite,
    drifter_animations[model->action],
    12 /* fps */, 
    0 /* starting_frame */
  );
  sprite_animator_start(a->animator);
  
  return NULL;
}

void* drifter_despawn(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  drifter* a = (drifter*)self;
  if (!a->sprite) {
    api->system->error("Cannot despawn drifter that is already despawned");
  }

  api->sprite->freeSprite(a->sprite);
  a->sprite = NULL;
  sprite_animator_destroy(a->animator);
  a->animator = NULL;

  return NULL;
}

drifter* drifter_create(world* w, controls* c, point* p, int cheat) {
  load_animations_if_needed();
  PlaydateAPI* api = get_api();
  drifter* a = malloc(sizeof(drifter));
  if (!a) {
    get_api()->system->error("Could not allocate memory for drifter");
  }

  drifter_model initial_extended = {
    .action = cheat,
  };
  int_rect bounds = { 
    .x = p->x, 
    .y = p->y, 
    .width = DRIFTER_WIDTH_PX, 
    .height = DRIFTER_HEIGHT_PX 
  };
  entity_behavior behavior = {
    .spawn = closure_create(a, drifter_spawn),
    .show = closure_create(a, drifter_show),
    .apply = closure_create(a, drifter_apply),
    .despawn = closure_create(a, drifter_despawn),
    .plan = closure_create(a, drifter_plan)
  };
  a->self = entity_create(
    DRIFTER_LABEL,
    &bounds,
    &initial_extended,
    &behavior,
    drifter_model_allocator,
    drifter_model_destructor,
    drifter_model_copy
  );

  a->sprite = NULL;
  a->animator = NULL;

  world_add_entity(w, a->self);

  return a;
}

void drifter_destroy(drifter* a) {
  PlaydateAPI* api = get_api();
  entity_destroy(a->self);  
  sprite_animator_destroy(a->animator);
  api->sprite->removeSprite(a->sprite);
  api->sprite->freeSprite(a->sprite);
  free(a);
}
