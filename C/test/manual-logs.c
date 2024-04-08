#include "pd_api.h"

#include "C/core/api-provider.h"
#include "C/core/fps-timers.h"

#include "manual-logs.h"

static pretend_animation test[] = {
  { .fps = 1, .label = "1-frame" },
  { .fps = 2, .label = "2-frame-a" },
  { .fps = 30, .label = "every-frame" },
  { .fps = 2, .label = "2-frame-b" },
  { .fps = 2, .label = "2-frame-c" },
  { .fps = 12, .label = "12-frame" },
};
static uint32_t timer_ids[6];

void advance_animation(void* animation) {
  pretend_animation* a = (pretend_animation*)animation;
  get_api()->system->logToConsole(
    "Pretend animation \"%s\" advanced at  %d fps",
    a->label,
    a->fps
  );
}

static uint32_t killer_id = 7;
static uint32_t kill_target_id = UINT32_MAX;
static uint8_t kill_target_fps = 0;
static int delay; 
void kill_animation(void* _) {
  delay--;
  if (delay > 0) {
    get_api()->system->logToConsole("killer has %d more rounds to wait...", delay);
    return;
  } else if (delay < 0) {
    get_api()->system->logToConsole("killer lingers as a corrupted ghost...");
    return;
  }

  if (kill_target_fps == 0 || kill_target_id == UINT32_MAX) {
    get_api()->system->logToConsole("killer struck at no target...", delay);
  } else {
    if (kill_target_fps == 1 && kill_target_id == killer_id) {
      get_api()->system->logToConsole("killer self destructs");
    } else {
      get_api()->system->logToConsole("killer strikes { id:%d, fps: %d }!", kill_target_id, kill_target_fps);
    }
    fps_timer_stop(kill_target_id, kill_target_fps);
  }
  kill_target_id = killer_id;
  kill_target_fps = 1;
  delay = 1;
}

void run_tests(void) {
  PlaydateAPI* api = get_api();

  for (int i = 0; i < 6; i++) {
    timer_ids[i] = fps_timer_start(&advance_animation, &test[i], test[i].fps, true);
  }

  kill_target_id = timer_ids[2];
  kill_target_fps = 30;
  delay = 2; 
  killer_id = fps_timer_start(
    &kill_animation,
    NULL,
    1 /* fps */,
    true /* loop */
  );

  get_api()->system->logToConsole("Timers setup, with function pointers { advance_animation:%p, kill_animation:%p }", advance_animation, kill_animation);
}
