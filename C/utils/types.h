
#ifndef UTIL_TYPES
#define UTIL_TYPES

#include <stdint.h>

typedef void* (*allocator_fn)(void);
typedef void (*destructor_fn)(void *);
typedef void (*copy_fn)(void* destination, void* source);

typedef struct grid_pos_struct {
  int row;
  int col;
} grid_pos;

typedef uint16_t gid_t;

static const gid_t INVALID_GID = UINT16_MAX;

// Guaranteed to monotonically increase up to INVALID_GID
gid_t getNextGID(void);

#endif
