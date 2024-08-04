
#include "C/api.h"

#include "logging.h"

void log_rect(PDRect r) {
  get_api()->system->logToConsole(
    "{ x: %f, y: %f, width: %f, height: %f }", 
    r.x, 
    r.y, 
    r.width, 
    r.height
  );
}
