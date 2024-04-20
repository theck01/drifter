
#ifndef UTIL_TYPES
#define UTIL_TYPES

#include <stdint.h>

typedef uint16_t gid_t;

static const gid_t INVALID_GID = UINT16_MAX;

// Guaranteed to monotonically increase up to INVALID_GID
gid_t getNextGID(void);

#endif
