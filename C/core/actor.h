
#ifndef ACTOR
#define ACTOR

uint8_t HISTORY_SIZE = 12;

struct Delta {
  struct {
    float x;
    float y;
  } pos;
}

struct Actor {
  DeltaPool pool;
  struct {
    Delta pool[HISTORY_SIZE];
    uint8_t index;
  } deltaPool;
};

#endif
