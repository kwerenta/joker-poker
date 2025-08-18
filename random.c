#include "random.h"

#include <stdlib.h>
#include <time.h>

void rng_init() { srand(time((NULL))); }

int16_t random_filtered_range_pick(uint8_t start, uint8_t end, RangeFilter filter) {
  if (start > end || end == 0) return -1;

  uint8_t candidates[end - start + 1];
  uint8_t count = 0;

  for (uint8_t i = start; i <= end; i++) {
    if (!filter || filter(i)) candidates[count++] = i;
  }

  if (count == 0) return -1;

  return candidates[rand() % count];
}

int16_t random_weighted(uint8_t *weights, uint8_t count) {
  if (weights == NULL || count == 0) return -1;

  float total_weight = 0.0f;
  for (uint8_t i = 0; i < count; i++) {
    total_weight += weights[i];
  }

  if (total_weight <= 0) return -1;

  float random_value = ((double)rand() / RAND_MAX) * total_weight;

  total_weight = 0.0f;
  for (uint8_t i = 0; i < count; i++) {
    total_weight += weights[i];
    if (random_value <= total_weight) return i;
  }

  return -1;
}

bool random_chance(uint8_t numerator, uint8_t denominator) {
  if (numerator <= 0 || denominator <= 0) return 0;
  if (numerator >= denominator) return 1;

  uint8_t result = rand() % denominator;
  return result < numerator;
}

uint8_t random_max_value(uint8_t max_value) { return rand() % max_value; }

uint8_t random_in_range(uint8_t min_value, uint8_t max_value) { return random_max_value(max_value) + min_value; }
