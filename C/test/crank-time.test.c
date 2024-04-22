
#include "C/api.h"
#include "C/core/crank-time.h"
#include "C/utils/closure.h"

void log_crank_diff(void* context, va_list args) {
  int time_diff = va_arg(args, int);
  get_api()->system->logToConsole("Crank changed by %d ticks", time_diff);
}

void crank_time_run_tests(void) {
  crank_time_add_listener(
    closure_create(NULL /* context */, log_crank_diff, NULL /* cleanup */)
  );
}
