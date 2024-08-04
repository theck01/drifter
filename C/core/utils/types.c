
#include "C/api.h"

#include "types.h"

static gid_t next_gid = 0;

gid_t getNextGID(void) {
  if (next_gid == INVALID_GID) {
    get_api()->system->error("Consumed all available gid_t values");
  }
  return next_gid++;
}
