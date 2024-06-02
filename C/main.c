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
#include "C/actors/ant.h"
#include "C/core/crank-time.h"
#include "C/core/fps-timers.h"
#include "C/ui/history-gauge.h"
#include "C/ui/viewport.h"
#include "C/ui/map-grid.h"
#include "C/utils/random.h"
#include "C/utils/types.h"
#include "C/utils/vector.h"

static vector* ant_vector = NULL;
static controls* default_controls = NULL;
static PlaydateAPI* api = NULL;

int update_loop(void* _) {
  controls_poll(default_controls);
  crank_time_update();
  fps_timers_update();
  api->sprite->updateAndDrawSprites();
  api->system->drawFPS(0, 0);
  return 1;
}

int destroy_ants(lua_State* L) {
  ant* a = (ant*)vector_pop(ant_vector);
  while (a) {
    ant_destroy(a);
    a = (ant*)vector_pop(ant_vector);
  }
  vector_destroy(ant_vector);
  api->lua->pushNil();
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

    ant_vector = vector_create(100);
    for (uint8_t i=0; i<100; i++) {
      ant* a = ant_create(random_uint(20, 380), random_uint(20, 220));
      vector_push(ant_vector, a);
    }

    default_controls = create_controls();
    history_gauge_connect();
    map_grid_show();
    viewport_connect(default_controls);
  } 
  return 0;
}
