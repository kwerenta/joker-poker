#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

typedef uint8_t (*RangeFilter)(uint8_t index);

void rng_init();

int16_t random_filtered_range_pick(uint8_t start, uint8_t end, RangeFilter filter);
int16_t random_weighted(uint8_t *weights, uint8_t count);
uint8_t random_chance(uint8_t numerator, uint8_t denominator);

#endif
