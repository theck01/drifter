
#ifndef GAME_CONSTANTS
#define GAME_CONSTANTS

#include <stdint.h>

static const uint8_t BUTTON_COUNT = 6;
static const uint8_t INPUT_QUEUE_SIZE = 8;

static const uint8_t CURRENT_FPS = 30;
static const uint8_t MAX_FPS = 50;
static const uint8_t CRANK_TICKS_PER_REVOLUTION = 30;
static const uint16_t HISTORY_SIZE = 300;

// Must be divisible by screen size
static const int SCREEN_WIDTH_PX = 400;
static const int SCREEN_HEIGHT_PX = 240;
static const uint8_t MAP_TILE_SIZE_PX = 40;

static const uint16_t MAP_GRID_Z_INDEX = 10;
static const uint16_t ACTOR_Z_INDEX = 50;
static const uint16_t HUD_Z_INDEX = 100;


#endif
