//
//  main.c
//  Extension
//
//  Created by Dan Messing on 5/01/18.
//  Copyright (c) 2018 Panic, Inc. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pd_api.h"

#include "C/api.h"
#include "C/core/controls.h"
#include "C/core/game-clock.h"
#include "C/core/fps-timers.h"
#include "C/core/input-generator.h"
#include "C/core/sprite-animator.h"
#include "C/core/structure.h"
#include "C/core/viewport.h"
#include "C/core/world.h"
#include "C/entities/active/drifter.h"
#include "C/ui/map-grid.h"
#include "C/utils/random.h"
#include "C/utils/vector.h"

static PlaydateAPI* api = NULL;

static world* main_world = NULL;
static controls* default_controls = NULL;
static drifter* player = NULL;
gid_t animation_listener_id = INVALID_GID;

void* game_speed_animate(void* _, va_list args) {
  int current_time = va_arg(args, int);
  clock_mask_e clock_mask = (clock_mask_e)va_arg(args, int);
  // == specifically to mean that it is not START | END;
  if (clock_mask == START) {
    sprite_animator_global_pause();
  }

  fps_timers_update();

  // == specifically to mean that it is not START | END;
  if (clock_mask == END) {
    sprite_animator_global_resume();
  }
  return NULL;
}

int update_loop(void* _) {
  input_generator_flush(default_controls);
  game_clock_update();
  api->sprite->updateAndDrawSprites();
  api->system->drawFPS(0, 0);
  return 1;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(
  PlaydateAPI* playdate, 
  PDSystemEvent event, 
  uint32_t arg
) {
  (void)arg;

  if (event == kEventInit) {
    set_api(playdate);
    api = playdate;
    srand(playdate->system->getSecondsSinceEpoch(NULL));
    playdate->system->resetElapsedTime();
    playdate->system->setUpdateCallback(update_loop, NULL);
    input_generator_listen();

    main_world = world_create(30, 18);
    default_controls = create_controls();
    point p = { .x = 600, .y = 120 };
    player = drifter_create(main_world, default_controls, &p);

    map_grid_show();

    viewport_connect(default_controls);
    viewport_set_offset(400, 0);

    game_clock_add_listener(closure_create(NULL, game_speed_animate));

    // Redraw all sprites on every frame and avoid dirty rect tracking.
    // The number of sprites planned to be moving around screen causes the
    // performance dirty rects to drop significantly while drawing on every
    // frame remains performant (especially with other methods of reducing
    // draw counts)
    api->sprite->setAlwaysRedraw(1);
  } 
  return 0;
}
