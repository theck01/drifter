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
#include "C/test/manual-logs.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(
  PlaydateAPI* playdate, 
  PDSystemEvent event, 
  uint32_t arg
) {
	(void)arg;

	if ( event == kEventInitLua )
	{
    set_api(playdate);
    playdate->system->resetElapsedTime();
    run_tests();
	}

	return 0;
}
