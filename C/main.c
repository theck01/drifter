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
#include "C/utils/random.h"
#include "C/utils/vector.h"

int c_update_loop(lua_State *L) {
  crank_time_update();
  fps_timers_update();
  get_api()->lua->pushNil();
  return 1;
}

static vector* ant_vector = NULL;
static PlaydateAPI* api = NULL;

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
    api = NULL;
    srand(playdate->system->getSecondsSinceEpoch(NULL));
    playdate->system->resetElapsedTime();
    // run_tests();
	} 
  else if (event == kEventInitLua) {
		const char* err;
		if (
      !playdate->lua->addFunction(
        c_update_loop, 
        "cupdate", 
        &err
      )
    ) {
			playdate->system->logToConsole("%s:%i: addFunction failed, %s", __FILE__, __LINE__, err);
    }

    ant_vector = vector_create(100);
    for (uint8_t i=0; i<100; i++) {
      ant* a = ant_create(random_uint(20, 380), random_uint(20, 220));
      vector_push(ant_vector, a);
    }
  }  

	return 0;
}
