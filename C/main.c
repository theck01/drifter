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
#include "C/core/sprite-animator.h"
#include "C/core/fps-timers.h"
#include "C/test/fps-timers.test.h"

void update_sprite(LCDSprite* s) {
  get_api()->sprite->setDrawMode(s, kDrawModeInverted);
}

int c_update_loop(lua_State *L) {
  fps_timers_update();
  get_api()->lua->pushNil();
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

    LCDSprite* ant_sprite = playdate->sprite->newSprite();
    playdate->sprite->setUpdateFunction(ant_sprite, &update_sprite);
    playdate->sprite->moveTo(ant_sprite, 100, 210);
    playdate->sprite->setZIndex(ant_sprite, 1000);


    LCDBitmapTable* ant_idle_animation = playdate->graphics->loadBitmapTable(
      "img/ant-idle-right.gif",
      &err
    );
    if (!ant_idle_animation) {
      playdate->system->error("Could not load animation: %s", err);
    }
    sprite_animator* ant_animator = sprite_animator_create(
      ant_sprite,
      ant_idle_animation, 
      12 /* fps */, 
      0 /* starting_frame */
    );

    playdate->sprite->addSprite(ant_sprite);
    sprite_animator_start(ant_animator);
    fps_timers_run_tests();
  }  

	return 0;
}
