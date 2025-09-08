#include "random.h"

#include <stdlib.h>
#include <time.h>

#include "content/joker.h"
#include "cvector.h"
#include "game.h"
#include "state.h"

void rng_init() { srand(time((NULL))); }

int16_t random_filtered_range_pick(uint8_t start, uint8_t end, RangeFilter filter) {
  if (start > end) return -1;

  uint8_t candidates[end - start + 1];
  uint8_t count = 0;

  for (uint8_t i = start; i <= end; i++) {
    if (!filter || filter(i)) candidates[count++] = i;
  }

  if (count == 0) return -1;

  return candidates[random_max_value(count - 1)];
}

int16_t random_filtered_vector_pick(cvector_vector_type(void) vec, RangeFilter filter) {
  if (cvector_size(vec) <= 0) return -1;
  return random_filtered_range_pick(0, cvector_size(vec) - 1, filter);
}

int16_t random_weighted(uint16_t *weights, uint8_t count) {
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

bool random_percent(float probability) {
  if (probability <= 0.0) return false;
  if (probability >= 1.0) return true;

  float random_value = rand() / (float)RAND_MAX;
  return random_value < probability;
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

Joker random_weighted_joker(uint16_t rarity_weights[4]) {
  uint16_t weights[JOKER_COUNT];
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

  // TODO According to Wiki "Joker" is returned when there are no more Jokers available and this should be changed when
  // more jokers will be added
  // Allow duplicates if there are no more jokers available
  if (!has_any_weights)
    for (uint8_t i = 0; i < JOKER_COUNT; i++) weights[i] = rarity_weights[JOKERS[i].rarity];

  Joker joker = JOKERS[random_weighted(weights, JOKER_COUNT)];

  // Base, Foil, Holographic, Polychrome, Negative
  uint16_t edition_weights[] = {960, 20, 14, 3, 3};
  for (uint8_t i = 1; i < 4; i++) {
    uint8_t multiplier = 1;
    if (state.game.vouchers & VOUCHER_GLOW_UP)
      multiplier = i == 3 ? 7 : 4;
    else if (state.game.vouchers & VOUCHER_HONE)
      multiplier = i == 3 ? 3 : 2;

    edition_weights[0] -= (multiplier - 1) * edition_weights[i];
    edition_weights[i] *= multiplier;
  }
  joker.edition = random_weighted(edition_weights, 5);

  // None, Eternal, Perishable
  uint16_t sticker_weights[] = {100, 0, 0};
  if (state.game.stake >= STAKE_BLACK) {
    sticker_weights[0] -= 30;
    sticker_weights[1] += 30;
  }
  if (state.game.stake >= STAKE_ORANGE) {
    sticker_weights[0] -= 30;
    sticker_weights[2] += 30;
  }
  joker.sticker = random_weighted(sticker_weights, 3);

  if (state.game.stake >= STAKE_GOLD && random_percent(0.3)) joker.sticker |= STICKER_RENTAL;

  return joker;
}

Joker random_available_joker() {
  // Common, Uncommon, Rare, Legendary
  uint16_t rarity_weights[] = {70, 25, 5, 0};
  return random_weighted_joker(rarity_weights);
}

Joker random_available_joker_by_rarity(Rarity rarity) {
  // Common, Uncommon, Rare, Legendary
  uint16_t base_rarity_weights[] = {70, 25, 5, 0};
  uint16_t rarity_weights[4] = {0};
  rarity_weights[rarity] = base_rarity_weights[rarity];
  return random_weighted_joker(rarity_weights);
}

Card random_card() {
  uint16_t edition_weights[5] = {920, 12, 28, 40, 0};
  for (uint8_t i = 1; i < 4; i++) {
    uint8_t multiplier = (state.game.vouchers & VOUCHER_GLOW_UP) ? 4 : (state.game.vouchers & VOUCHER_HONE) ? 2 : 1;
    edition_weights[0] -= (multiplier - 1) * edition_weights[i];
    edition_weights[i] *= multiplier;
  }
  Edition edition = random_weighted(edition_weights, 5);
  Enhancement enhancement = ENHANCEMENT_NONE;
  Seal seal = SEAL_NONE;

  if (random_chance(4, 10)) enhancement = random_in_range(ENHANCEMENT_BONUS, ENHANCEMENT_LUCKY);
  if (random_chance(2, 10)) seal = random_in_range(SEAL_GOLD, SEAL_PURPLE);

  return create_card(random_max_value(3), random_max_value(12), edition, enhancement, seal);
}

Card random_shop_card() {
  if (!(state.game.vouchers & VOUCHER_ILLUSION))
    return create_card(random_max_value(3), random_max_value(12), EDITION_BASE, ENHANCEMENT_NONE, SEAL_NONE);

  Card card = random_card();
  card.edition = EDITION_BASE;

  if (random_chance(2, 10)) card.edition = random_in_range(EDITION_FOIL, EDITION_POLYCHROME);

  return card;
}
