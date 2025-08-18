#ifndef RANDOM_H
#define RANDOM_H

#include <stdbool.h>
#include <stdint.h>

typedef bool (*RangeFilter)(uint8_t index);

void rng_init();

int16_t random_filtered_range_pick(uint8_t start, uint8_t end, RangeFilter filter);
int16_t random_weighted(uint8_t *weights, uint8_t count);
bool random_chance(uint8_t numerator, uint8_t denominator);

#endif
