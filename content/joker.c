#include "../state.h"

void activate_joker_1() { state.game.selected_hand.scoring.mult += 4; }

void activate_joker_6() {
  if (does_poker_hand_contain(state.game.selected_hand.hand_union, HAND_PAIR))
    state.game.selected_hand.scoring.mult += 8;
}

const Joker JOKERS[] = {{.id = 1,
                         .name = "Joker",
                         .description = "+4 mult when scored",
                         .base_price = 3,
                         .rarity = RARITY_COMMON,
                         .activation_type = ACTIVATION_INDEPENDENT,
                         .activate = activate_joker_1},
                        {.id = 6,
                         .name = "Jolly Joker",
                         .description = "+8 mult when scored hand contains pair",
                         .base_price = 5,
                         .rarity = RARITY_COMMON,
                         .activation_type = ACTIVATION_INDEPENDENT,
                         .activate = activate_joker_6}};

const uint8_t JOKER_COUNT = sizeof(JOKERS) / sizeof(Joker);
