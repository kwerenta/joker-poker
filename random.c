#include "random.h"

#include <stdlib.h>
#include <time.h>

void rng_init() { srand(time((NULL))); }

static int16_t random_filtered_range_pick(uint8_t start, uint8_t end, RangeFilter filter) {
  uint8_t candidates[end - start];
  uint8_t count = 0;

  for (uint8_t i = start; i < end; i++) {
    if (!filter || filter(i)) candidates[count++] = i;
  }

  if (count == 0) return -1;

  return candidates[rand() % count];
}
