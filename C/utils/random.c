
#include <stdlib.h>

#include "C/api.h"
#include "random.h"

float randomf(void) {
  int n = rand();
  return ((float)n / (float)RAND_MAX);
}

uint16_t random_uint(uint16_t min, uint16_t max) {
  float frand = randomf();
  return min + (uint16_t)(frand * (max - min));
}
