
#ifndef ANT_ACTOR
#define ANT_ACTOR

typedef struct ant_struct ant;

ant* ant_create(int x, int y);

void ant_destroy(ant* ant);

#endif
