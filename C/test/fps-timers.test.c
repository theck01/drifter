#include "pd_api.h"

#include "C/api.h"
#include "C/core/fps-timers.h"
#include "C/utils/closure.h"

#include "fps-timers.test.h"

typedef struct pretend_animation_struct {
  uint8_t fps;
  char* label;
} pretend_animation;

static pretend_animation test[] = {
  { .fps = 1, .label = "1-frame" },
  { .fps = 2, .label = "2-frame-a" },
  { .fps = 30, .label = "every-frame" },
  { .fps = 2, .label = "2-frame-b" },
  { .fps = 2, .label = "2-frame-c" },
  { .fps = 4, .label = "4-frame" },
  { .fps = 12, .label = "12-frame" },
};
static uint32_t timer_ids[7];

static pretend_animation late_start[] = {
  { .fps = 4, .label = "4-frame-rebirth" },
  { .fps = 8, .label = "8-frame" },
  { .fps = 16, .label = "16-frame" }
};

void* advance_animation(void* animation, va_list _) {
  pretend_animation* a = (pretend_animation*)animation;
  get_api()->system->logToConsole(
    "Pretend animation \"%s\" advanced at  %d fps",
    a->label,
    a->fps
  );
  return NULL;
}

static uint8_t stage_n = 1;
static uint32_t killer_id = 7;
static int delay = 2; 
void* kill_animation(void* context, va_list args) {
  delay--;
  if (delay > 0) {
    get_api()->system->logToConsole("killer has %d more rounds to wait...", delay);
    return NULL;
  } else if (delay < 0) {
    get_api()->system->logToConsole("killer lingers as a corrupted ghost...");
    return NULL;
  }

  switch (stage_n) {
    case 1:
      get_api()->system->logToConsole("killer strikes { id:%d, fps: %d }!", timer_ids[2], 30);
      fps_timer_stop(30, timer_ids[2]);
      get_api()->system->logToConsole("killer strikes { id:%d, fps: %d }!", timer_ids[5], 4);
      fps_timer_stop(4, timer_ids[5]);
      delay = 2;
      break;
    case 2:
      get_api()->system->logToConsole("killer creates new animations");
      for (uint32_t i = 0; i < 3; i++) {
        fps_timer_start(
          late_start[i].fps,
          closure_create(&late_start[i], advance_animation)
        );
      }
      delay = 1;
      break;
    default:
      get_api()->system->logToConsole("killer self destructs");
      fps_timer_stop(1, killer_id);
      break;
  }
  stage_n++;
  return NULL;
}

void fps_timers_run_tests(void) {
  PlaydateAPI* api = get_api();

  for (int i = 0; i < 7; i++) {
    timer_ids[i] = fps_timer_start(
      test[i].fps,
      closure_create(&test[i], advance_animation)
    );
  }
  killer_id = fps_timer_start(
    1 /* fps */,
    closure_create(NULL /* context */, kill_animation)
  );

  get_api()->system->logToConsole("Timers setup, with function pointers { advance_animation:%p, kill_animation:%p }", advance_animation, kill_animation);
}
