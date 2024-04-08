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

#include "C/core/api-provider.h"
#include "C/core/fps-timers.h"
#include "C/test/manual-logs.h"

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
    run_tests();
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
  }  

	return 0;
}
