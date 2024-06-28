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
#include "C/core/crank-time.h"
#include "C/core/fps-timers.h"
#include "C/core/input-generator.h"
#include "C/core/structure.h"
#include "C/core/viewport.h"
#include "C/core/world.h"
#include "C/entities/active/ant.h"
#include "C/ui/history-gauge.h"
#include "C/ui/map-grid.h"
#include "C/utils/random.h"
#include "C/utils/vector.h"

const int ANT_COUNT = 200;

static world* main_world = NULL;
static vector* ant_vector = NULL;
static controls* default_controls = NULL;
static PlaydateAPI* api = NULL;

int update_loop(void* _) {
  input_generator_flush(default_controls);
  crank_time_update();
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
    input_generator_listen();

    main_world = world_create(30, 18);

    ant_vector = vector_create(ANT_COUNT);
    for (uint8_t i=0; i<ANT_COUNT; i++) {
      ant* a = ant_create(
        main_world, 
        random_uint(420, 780), 
        random_uint(20, 220)
      );
      vector_push(ant_vector, a);
    }

    default_controls = create_controls();
    history_gauge_connect();
    map_grid_show();
    viewport_connect(default_controls);
    viewport_set_offset(400, 0);

    // Redraw all sprites on every frame and avoid dirty rect tracking.
    // The number of sprites planned to be moving around screen causes the
    // performance dirty rects to drop significantly while drawing on every
    // frame remains performant (especially with other methods of reducing
    // draw counts)
    api->sprite->setAlwaysRedraw(1);
  } 
  return 0;
}
