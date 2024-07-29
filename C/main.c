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
#include "C/core/camera.h"
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
static camera* main_camera = NULL;
static drifter* player = NULL;

int update_loop(void* _) {
  input_generator_flush(default_controls);
  game_clock_update();
  fps_timers_update();
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
    // Moving animating sprites while scrolling the screen causes severe
    // framerate issues when attempting to be smart about what sprites
    // need to be redrawn. Avoid this performance cost by drawing everything
    // every frame, to skip the occlusion logic.
    playdate->sprite->setAlwaysRedraw(1);


    input_generator_listen();

    main_world = world_create(30, 18);
    default_controls = create_controls();

    point p = { .x = 600, .y = 120 };
    player = drifter_create(main_world, default_controls, &p);

    point camera_origin = { .x = 400, .y = 0 };
    main_camera = camera_create(main_world, camera_origin);
    camera_track(main_camera, drifter_get_entity(player));

    map_grid_show();
  } 
  return 0;
}
