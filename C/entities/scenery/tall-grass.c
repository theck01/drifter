
#include "C/api.h"
#include "C/core/graphics/sprite.h"
#include "C/core/graphics/sprite-animator.h"
#include "C/core/world/entity.h"

#include "tall-grass.h"

static const char* TALL_GRASS_LABEL = "tall_grass";
static const point TALL_GRASS_SIZE = { .x = 28, .y = 24 };

struct tall_grass_struct {
  entity* self;
  LCDSprite* sprite;
  sprite_animator* animator;
};

static LCDBitmapTable* grass_gif = NULL;

void load_img_if_needed(void) {
  if (grass_gif) {
    return;
  }

  PlaydateAPI* api = get_api();
  const char* err;
  grass_gif = api->graphics->loadBitmapTable(
    "img/scenery/tall-grass.gif", 
    &err
  );
  if (!grass_gif) {
    api->system->error("Could not load tall grass gif, err: %s", err);
  }
}

// Model stub, should react to presence of nearby entities in the future
void* tall_grass_model_create(void) {
  return NULL;
}
void tall_grass_model_destroy(void* model) {}
void tall_grass_model_copy(void* destination, const void* source) {}

// Behavior
//
void* tall_grass_spawn(void* context, va_list args) { 
  load_img_if_needed();

  PlaydateAPI* api = get_api();
  tall_grass* tg = (tall_grass*)context;

  if (tg->sprite) {
    api->system->error("Cannot spawn tall_grass that already has spawned");
  }

  point p;
  entity_get_position(tg->self, &p);

  tg->sprite = create_entity_sprite();
  api->sprite->moveTo(tg->sprite, p.x, p.y);
  api->sprite->addSprite(tg->sprite);
  api->sprite->setVisible(tg->sprite, false);

  tg->animator = sprite_animator_create(
    tg->sprite,
    grass_gif,
    5 /* fps */, 
    0 /* starting_frame */
  );
  sprite_animator_start(tg->animator);

  return NULL;
}

void* tall_grass_show(void* self, va_list args) {
  PlaydateAPI* api = get_api();
  tall_grass* tg = (tall_grass*)self;
  int show = (bool)va_arg(args, int);
  api->sprite->setVisible(tg->sprite, show);
  if (show) {
    sprite_animator_resume(tg->animator);
  } else {
    sprite_animator_pause(tg->animator);
  }
  return NULL;
}

void* tall_grass_apply(void* context, va_list args) {
  return NULL;
}

void* tall_grass_despawn(void* context, va_list args) {
  PlaydateAPI* api = get_api();
  tall_grass* tg = (tall_grass*)context;

  if (!tg->sprite) {
    api->system->error("Cannot despawn tall_grass that is not spawned");
  }

  api->sprite->removeSprite(tg->sprite);
  api->sprite->freeSprite(tg->sprite);
  tg->sprite = NULL;

  sprite_animator_destroy(tg->animator);
  tg->animator = NULL;

  return NULL;
}

// API

tall_grass* tall_grass_create(world* w, point* position) {
  tall_grass* tg = malloc(sizeof(tall_grass));
  if (!tg) {
    get_api()->system->error("Cannot allocate memory for tall_grass"); 
  }

  entity_behavior behavior = {
    .spawn = closure_create(tg, tall_grass_spawn),
    .apply = closure_create(tg, tall_grass_apply),
    .show = closure_create(tg, tall_grass_show),
    .despawn = closure_create(tg, tall_grass_despawn),
    .plan = NULL
  };
  tg->self = entity_create(
    TALL_GRASS_LABEL,
    position,
    &TALL_GRASS_SIZE,
    NULL,
    &behavior,
    tall_grass_model_create,
    tall_grass_model_destroy,
    tall_grass_model_copy
  );

  tg->sprite = NULL;
  tg->animator = NULL;

  world_add_entity(w, tg->self);

  return tg;
}

void tall_grass_destroy(tall_grass* tg) {
  entity_destroy(tg->self);  
  free(tg);
}
