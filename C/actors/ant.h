
#ifndef ANT_ACTOR
#define ANT_ACTOR

typedef struct ant_struct ant;

ant* ant_create(float x, float y);

void ant_destroy(ant* ant);

#endif
