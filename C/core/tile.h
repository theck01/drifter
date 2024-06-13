
#ifndef TILE
#define TILE

#include <stdbool.h>

#include "entity.h"

typedef struct tile_struct tile;

typedef struct tile_metadata_struct {
  int row;
  int col;
  bool shown;
} tile_metadata;


void tile_get_metadata(tile* t, tile_metadata* tmd);

void tile_add_entity(tile* t, entity* e);

void tile_remove_entity(tile* t, entity* e);

void tile_show(tile* t, bool show);


#endif
