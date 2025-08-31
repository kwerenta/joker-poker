#include "joker.h"

#include "../state.h"

static void activate_joker_joker(Joker *self) { state.game.selected_hand.score_pair.mult += 4; }

static void activate_joker_jolly(Joker *self) {
  if (does_poker_hand_contain(state.game.selected_hand.hand_union, HAND_PAIR))
    state.game.selected_hand.score_pair.mult += 8;
}

static void activate_greedy_joker(Joker *self, Card *card) {
  if (card->suit == SUIT_DIAMONDS) state.game.selected_hand.score_pair.mult += 3;
}

static void activate_lusty_joker(Joker *self, Card *card) {
  if (card->suit == SUIT_HEARTS) state.game.selected_hand.score_pair.mult += 3;
}

static void activate_wrathful_joker(Joker *self, Card *card) {
  if (card->suit == SUIT_SPADES) state.game.selected_hand.score_pair.mult += 3;
}

static void activate_gluttonous_joker(Joker *self, Card *card) {
  if (card->suit == SUIT_CLUBS) state.game.selected_hand.score_pair.mult += 3;
}

const Joker JOKERS[] = {
    {
        .id = JOKER_JOKER,
        .name = "Joker",
        .description = "+4 mult when scored",
        .base_price = 2,
        .rarity = RARITY_COMMON,
        .activate_independent = activate_joker_joker,
    },
    {
        .id = JOKER_GREEDY,
        .name = "Greedy Joker",
        .description = "Played cards with Diamond suit give +3 Mult when scored",
        .base_price = 5,
        .rarity = RARITY_COMMON,
        .activate_on_scored = activate_greedy_joker,
    },
    {
        .id = JOKER_LUSTY,
        .name = "Lusty Joker",
        .description = "Played cards with Heart suit give +3 Mult when scored",
        .base_price = 5,
        .rarity = RARITY_COMMON,
        .activate_on_scored = activate_lusty_joker,
    },
    {
        .id = JOKER_WRATHFUL,
        .name = "Wrathful Joker",
        .description = "Played cards with Spade suit give +3 Mult when scored",
        .base_price = 5,
        .rarity = RARITY_COMMON,
        .activate_on_scored = activate_wrathful_joker,
    },
    {
        .id = JOKER_GLUTTONOUS,
        .name = "Gluttonous Joker",
        .description = "Played cards with Club suit give +3 Mult when scored",
        .base_price = 5,
        .rarity = RARITY_COMMON,
        .activate_on_scored = activate_gluttonous_joker,
    },
    {
        .id = JOKER_JOLLY,
        .name = "Jolly Joker",
        .description = "+8 mult when scored hand contains pair",
        .base_price = 5,
        .rarity = RARITY_COMMON,
        .activate_independent = activate_joker_jolly,
    },
};

const uint8_t JOKER_COUNT = sizeof(JOKERS) / sizeof(Joker);
