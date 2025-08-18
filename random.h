#ifndef RANDOM_H
#define RANDOM_H

#include <stdbool.h>
#include <stdint.h>

#include "cvector.h"
#include "game.h"

#define random_vector_index(vec) random_max_value(cvector_size(vec) - 1)
#define random_vector_item(vec) vec[random_vector_index(vec)]

typedef bool (*RangeFilter)(uint8_t index);

void rng_init();

int16_t random_filtered_range_pick(uint8_t start, uint8_t end, RangeFilter filter);
int16_t random_weighted(uint8_t *weights, uint8_t count);
bool random_chance(uint8_t numerator, uint8_t denominator);
uint8_t random_max_value(uint8_t max_value);
uint8_t random_in_range(uint8_t min_value, uint8_t max_value);

Joker random_available_joker();
Joker random_available_joker_by_rarity(Rarity rarity);

#endif
