#include "joker.h"

#include "../state.h"

static void activate_joker_joker(Joker *self) { state.game.selected_hand.score_pair.mult += 4; }

static void activate_basic_suit_plus_mult(Joker *self, Card *card) {
  if (card->suit == self->suit) state.game.selected_hand.score_pair.mult += 3;
}

static void activate_basic_hand_plus_mult(Joker *self) {
  if (!does_poker_hand_contain(state.game.selected_hand.hand_union, self->hand)) return;

  uint8_t mult = 0;
  switch (self->hand) {
    case HAND_PAIR:
      mult = 8;
      break;
    case HAND_TWO_PAIR:
    case HAND_FLUSH:
      mult = 10;
      break;
    case HAND_THREE_OF_KIND:
    case HAND_STRAIGHT:
      mult = 12;
      break;
  }

  state.game.selected_hand.score_pair.mult += mult;
}

static void activate_basic_hand_plus_chips(Joker *self) {
  if (!does_poker_hand_contain(state.game.selected_hand.hand_union, self->hand)) return;

  uint8_t chips = 0;
  switch (self->hand) {
    case HAND_PAIR:
      chips = 50;
      break;
    case HAND_TWO_PAIR:
    case HAND_FLUSH:
      chips = 80;
      break;
    case HAND_THREE_OF_KIND:
    case HAND_STRAIGHT:
      chips = 100;
      break;
  }

  state.game.selected_hand.score_pair.chips += chips;
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
        .suit = SUIT_DIAMONDS,
        .activate_on_scored = activate_basic_suit_plus_mult,
    },
    {
        .id = JOKER_LUSTY,
        .name = "Lusty Joker",
        .description = "Played cards with Heart suit give +3 Mult when scored",
        .base_price = 5,
        .rarity = RARITY_COMMON,
        .suit = SUIT_HEARTS,
        .activate_on_scored = activate_basic_suit_plus_mult,
    },
    {
        .id = JOKER_WRATHFUL,
        .name = "Wrathful Joker",
        .description = "Played cards with Spade suit give +3 Mult when scored",
        .base_price = 5,
        .rarity = RARITY_COMMON,
        .suit = SUIT_SPADES,
        .activate_on_scored = activate_basic_suit_plus_mult,
    },
    {
        .id = JOKER_GLUTTONOUS,
        .name = "Gluttonous Joker",
        .description = "Played cards with Club suit give +3 Mult when scored",
        .base_price = 5,
        .rarity = RARITY_COMMON,
        .suit = SUIT_CLUBS,
        .activate_on_scored = activate_basic_suit_plus_mult,
    },
    {
        .id = JOKER_JOLLY,
        .name = "Jolly Joker",
        .description = "+8 mult when scored hand contains a Pair",
        .base_price = 3,
        .rarity = RARITY_COMMON,
        .hand = HAND_PAIR,
        .activate_independent = activate_basic_hand_plus_mult,
    },
    {
        .id = JOKER_ZANY,
        .name = "Zany Joker",
        .description = "+12 mult when scored hand contains a Three of a Kind",
        .base_price = 4,
        .rarity = RARITY_COMMON,
        .hand = HAND_THREE_OF_KIND,
        .activate_independent = activate_basic_hand_plus_mult,
    },
    {
        .id = JOKER_MAD,
        .name = "Mad Joker",
        .description = "+10 mult when scored hand contains a Two Pair",
        .base_price = 4,
        .rarity = RARITY_COMMON,
        .hand = HAND_TWO_PAIR,
        .activate_independent = activate_basic_hand_plus_mult,
    },
    {
        .id = JOKER_CRAZY,
        .name = "Crazy Joker",
        .description = "+12 mult when scored hand contains a Straight",
        .base_price = 4,
        .rarity = RARITY_COMMON,
        .hand = HAND_STRAIGHT,
        .activate_independent = activate_basic_hand_plus_mult,
    },
    {
        .id = JOKER_DROLL,
        .name = "Droll Joker",
        .description = "+10 mult when scored hand contains a Flush",
        .base_price = 4,
        .rarity = RARITY_COMMON,
        .hand = HAND_FLUSH,
        .activate_independent = activate_basic_hand_plus_mult,
    },
    {
        .id = JOKER_SLY,
        .name = "Sly Joker",
        .description = "+50 Chips if played hand contains a Pair",
        .base_price = 3,
        .rarity = RARITY_COMMON,
        .hand = HAND_PAIR,
        .activate_independent = activate_basic_hand_plus_chips,
    },
    {
        .id = JOKER_WILY,
        .name = "Wily Joker",
        .description = "+100 Chips if played hand contains a Three of a Kind",
        .base_price = 4,
        .rarity = RARITY_COMMON,
        .hand = HAND_THREE_OF_KIND,
        .activate_independent = activate_basic_hand_plus_chips,
    },
    {
        .id = JOKER_CLEVER,
        .name = "Clever Joker",
        .description = "+80 Chips if played hand contains a Two Pair",
        .base_price = 4,
        .rarity = RARITY_COMMON,
        .hand = HAND_TWO_PAIR,
        .activate_independent = activate_basic_hand_plus_chips,
    },
    {
        .id = JOKER_DEVIOUS,
        .name = "Devious Joker",
        .description = "+100 Chips if played hand contains a Straight",
        .base_price = 4,
        .rarity = RARITY_COMMON,
        .hand = HAND_STRAIGHT,
        .activate_independent = activate_basic_hand_plus_chips,
    },
    {
        .id = JOKER_CRAFTY,
        .name = "Craft Joker",
        .description = "+80 Chips if played hand contains a Flush",
        .base_price = 4,
        .rarity = RARITY_COMMON,
        .hand = HAND_FLUSH,
        .activate_independent = activate_basic_hand_plus_chips,
    },
};

const uint8_t JOKER_COUNT = sizeof(JOKERS) / sizeof(Joker);
