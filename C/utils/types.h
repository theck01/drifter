
#ifndef UTIL_TYPES
#define UTIL_TYPES

#include <stdint.h>

typedef void* (*allocator_fn)(void);
typedef void (*destructor_fn)(void *);
typedef void (*copy_fn)(void* destination, void* source);

// 0bLURD
typedef enum {
  U = 0b0100,
  UR = 0b0110,
  R = 0b0010,
  DR = 0b0011,
  D = 0b0001,
  DL = 0b1001,
  L = 0b1000,
  UL = 0b1100,
  NONE = 0b0000
} direction_e;

typedef struct grid_pos_struct {
  int row;
  int col;
} grid_pos;

typedef uint16_t gid_t;

static const gid_t INVALID_GID = UINT16_MAX;

// Guaranteed to monotonically increase up to INVALID_GID
gid_t getNextGID(void);

#endif
