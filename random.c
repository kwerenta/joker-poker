#include "random.h"

#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "state.h"

void rng_init() { srand(time((NULL))); }

int16_t random_filtered_range_pick(uint8_t start, uint8_t end, RangeFilter filter) {
  if (start >= end) return -1;

  uint8_t candidates[end - start + 1];
  uint8_t count = 0;

  for (uint8_t i = start; i <= end; i++) {
    if (!filter || filter(i)) candidates[count++] = i;
  }

  if (count == 0) return -1;

  return candidates[random_max_value(count)];
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

  return random_max_value(denominator - 1) < numerator;
}

uint8_t random_max_value(uint8_t max_value) { return random_in_range(0, max_value); }

uint8_t random_in_range(uint8_t min_value, uint8_t max_value) {
  return min_value + rand() / (RAND_MAX / (max_value - min_value + 1) + 1);
}

Joker random_weighted_joker(uint8_t rarity_weights[4]) {
  uint8_t weights[JOKER_COUNT];
  bool has_any_weights = false;

  for (uint8_t i = 0; i < JOKER_COUNT; i++) {
    bool has_this_joker = false;
    cvector_for_each(state.game.jokers.cards, Joker, joker) {
      if (joker->id == JOKERS[i].id) {
        has_this_joker = true;
        break;
      }
    }

    if (has_this_joker) {
      weights[i] = 0;
      continue;
    }

    weights[i] = rarity_weights[JOKERS[i].rarity];
    has_any_weights = true;
  };

  // Allow duplicates if there are no more jokers available
  if (!has_any_weights)
    for (uint8_t i = 0; i < JOKER_COUNT; i++) weights[i] = rarity_weights[JOKERS[i].rarity];

  return JOKERS[random_weighted(weights, JOKER_COUNT)];
}

Joker random_available_joker() {
  // Common, Uncommon, Rare, Legendary
  uint8_t rarity_weights[] = {70, 25, 5, 0};
  return random_weighted_joker(rarity_weights);
}

Joker random_available_joker_by_rarity(Rarity rarity) {
  // Common, Uncommon, Rare, Legendary
  uint8_t base_rarity_weights[] = {70, 25, 5, 0};
  uint8_t rarity_weights[4] = {0};
  rarity_weights[rarity] = base_rarity_weights[rarity];
  return random_weighted_joker(rarity_weights);
}
