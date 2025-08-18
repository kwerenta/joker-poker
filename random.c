#include "random.h"

#include <stdlib.h>
#include <time.h>

void rng_init() { srand(time((NULL))); }

int16_t random_filtered_range_pick(uint8_t start, uint8_t end, RangeFilter filter) {
  uint8_t candidates[end - start];
  uint8_t count = 0;

  for (uint8_t i = start; i < end; i++) {
    if (!filter || filter(i)) candidates[count++] = i;
  }

  if (count == 0) return -1;

  return candidates[rand() % count];
}

uint8_t random_chance(uint8_t numerator, uint8_t denominator) {
  if (numerator <= 0 || denominator <= 0) return 0;
  if (numerator >= denominator) return 1;

  uint8_t result = rand() % denominator;
  return result < numerator;
}
