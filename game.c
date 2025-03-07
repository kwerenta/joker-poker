#include <stdio.h>

#include "game.h"
#include "lib/cvector.h"
#include "state.h"

void activate_joker_1() { state.game.selected_hand.mult += 4; }
void activate_joker_6() {
  if (state.game.selected_hand.poker_hand == HAND_PAIR)
    state.game.selected_hand.mult += 8;
}

void game_init() {
  // Generate standard deck of 52 cards
  state.game.deck.size = 52;
  cvector_reserve(state.game.deck.cards, state.game.deck.size);
  for (uint8_t i = 0; i < state.game.deck.size; i++) {
    cvector_push_back(state.game.deck.cards, create_card(i % 4, i % 13));
  }

  state.game.full_deck.size = state.game.deck.size;
  cvector_copy(state.game.deck.cards, state.game.full_deck.cards);

  shuffle_deck();

  state.game.hand.size = 7;
  cvector_reserve(state.game.hand.cards, state.game.hand.size);
  fill_hand();

  state.game.money = 4;

  state.game.ante = 1;

  state.game.hands = 4;
  state.game.discards = 2;

  state.game.jokers.size = 5;
  ShopItem shop_item_1 = {.type = SHOP_ITEM_JOKER,
                          .price = 3,
                          .joker = {.id = 1,
                                    .name = "Joker",
                                    .description = "+4 mult when scored",
                                    .base_price = 3,
                                    .rarity = RARITY_COMMON,
                                    .activation_type = ACTIVATION_INDEPENDENT,
                                    .activate = activate_joker_1}};
  cvector_push_back(state.game.shop.items, shop_item_1);

  ShopItem shop_item_2 = {
      .type = SHOP_ITEM_JOKER,
      .price = 5,
      .joker = {.id = 6,
                .name = "Jolly Joker",
                .description = "+8 mult when scored hand is two pair",
                .base_price = 5,
                .rarity = RARITY_COMMON,
                .activation_type = ACTIVATION_INDEPENDENT,
                .activate = activate_joker_6}};
  cvector_push_back(state.game.shop.items, shop_item_2);

  ShopItem shop_item_3 = {.type = SHOP_ITEM_CARD,
                          .price = 1,
                          .card = create_card(SUIT_DIAMONDS, RANK_SEVEN)};
  cvector_push_back(state.game.shop.items, shop_item_3);
  state.game.shop.selected_card = 0;

  state.stage = STAGE_GAME;
}

void game_destroy() {
  cvector_free(state.game.deck.cards);
  cvector_free(state.game.full_deck.cards);
  cvector_free(state.game.hand.cards);
  cvector_free(state.game.jokers.cards);

  cvector_free(state.game.shop.items);
}

Card create_card(Suit suit, Rank rank) {
  uint16_t chips = rank == RANK_ACE ? 11 : rank + 1;
  if (rank != RANK_ACE && chips > 10) {
    chips = 10;
  }

  return (Card){.suit = suit, .rank = rank, .chips = chips, .selected = 0};
}

void draw_card() {
  cvector_push_back(
      state.game.hand.cards,
      state.game.deck.cards[cvector_size(state.game.deck.cards) - 1]);
  cvector_pop_back(state.game.deck.cards);
}

void fill_hand() {
  while (cvector_size(state.game.hand.cards) < state.game.hand.size) {
    draw_card();
  }
}

void play_hand() {
  if (state.game.hands == 0 || state.game.selected_hand.count == 0) {
    return;
  }

  update_scoring_hand();

  for (Joker *joker = cvector_begin(state.game.jokers.cards);
       joker != cvector_end(state.game.jokers.cards); joker++) {
    if (joker->activation_type == ACTIVATION_INDEPENDENT) {
      joker->activate();
      printf("Joker '%s' has been activated\n", joker->name);
    }
  }

  state.game.score +=
      state.game.selected_hand.chips * state.game.selected_hand.mult;

  remove_selected_cards();
  state.game.hands--;

  double required_score = get_required_score(state.game.ante, state.game.blind);

  if (state.game.score >= required_score) {
    state.stage = STAGE_SHOP;
    state.game.money += 1 * state.game.hands +
                        get_blind_money(state.game.blind) +
                        (state.game.money) / 5;
  } else if (state.game.hands == 0) {
    state.stage = STAGE_GAME_OVER;
  }
}

void discard_hand() {
  if (state.game.selected_hand.count == 0 || state.game.discards == 0) {
    return;
  }

  state.game.discards--;
  remove_selected_cards();
  fill_hand();
}

void remove_selected_cards() {
  uint8_t i = 0;
  while (i < cvector_size(state.game.hand.cards)) {
    if (state.game.hand.cards[i].selected == 1) {
      cvector_erase(state.game.hand.cards, i);
      continue;
    }

    i++;
  }

  state.game.selected_hand.count = 0;
}

void shuffle_deck() {
  const Deck *deck = &state.game.deck;
  for (uint8_t i = cvector_size(deck->cards) - 1; i > 0; i--) {
    uint8_t j = rand() % (i + 1);
    Card temp = deck->cards[i];
    deck->cards[i] = deck->cards[j];
    deck->cards[j] = temp;
  }
}

void set_hovered_card(uint8_t *hovered, uint8_t new_position) {
  if (new_position >= cvector_size(state.game.hand.cards)) {
    return;
  }

  *hovered = new_position;
}

void toggle_card_select(uint8_t index) {
  Hand *hand = &state.game.hand;

  if (hand->cards[index].selected == 1) {
    hand->cards[index].selected = 0;
    state.game.selected_hand.count--;
    update_scoring_hand();
    return;
  }

  uint8_t *selected_count = &state.game.selected_hand.count;
  *selected_count = 0;
  for (uint8_t i = 0; i < cvector_size(hand->cards); i++) {
    if (hand->cards[i].selected == 0) {
      continue;
    }

    (*selected_count)++;

    if (*selected_count == 5) {
      return;
    }
  }

  (*selected_count)++;

  hand->cards[index].selected = 1;

  update_scoring_hand();
}

void deselect_all_cards() {
  Hand *hand = &state.game.hand;
  for (uint8_t i = 0; i < cvector_size(hand->cards); i++) {
    hand->cards[i].selected = 0;
  }
  state.game.selected_hand.count = 0;
}

void move_card_in_hand(uint8_t *hovered, uint8_t new_position) {
  Hand *hand = &state.game.hand;

  if (new_position >= cvector_size(hand->cards)) {
    return;
  }

  Card temp = hand->cards[*hovered];
  hand->cards[*hovered] = hand->cards[new_position];
  hand->cards[new_position] = temp;

  *hovered = new_position;
}

int compare_by_rank(const void *a, const void *b) {
  Rank a_rank = ((const Card *)a)->rank, b_rank = ((const Card *)b)->rank;
  if (a_rank == RANK_ACE) {
    a_rank = RANK_KING + 1;
  }
  if (b_rank == RANK_ACE) {
    b_rank = RANK_KING + 1;
  }

  int by_rank = b_rank - a_rank;
  if (by_rank == 0) {
    return ((const Card *)a)->suit - ((const Card *)b)->suit;
  }

  return by_rank;
}
int compare_by_suit(const void *a, const void *b) {
  int by_suit = ((const Card *)a)->suit - ((const Card *)b)->suit;
  if (by_suit == 0) {
    return compare_by_rank(a, b);
  }

  return by_suit;
}

void sort_hand(uint8_t by_suit) {
  int (*comparator)(const void *a, const void *b) = compare_by_rank;

  if (by_suit == 1) {
    comparator = compare_by_suit;
  }

  qsort(state.game.hand.cards, cvector_size(state.game.hand.cards),
        sizeof(Card), comparator);
}

PokerHand evaluate_hand() {
  const Hand *hand = &state.game.hand;

  uint8_t rank_counts[13] = {};
  uint8_t suit_counts[4] = {};
  const uint8_t selected_count = state.game.selected_hand.count;

  // 2 of kind, 3 of kind, 4 of kind, 5 of kind
  uint8_t x_of_kind[4] = {};
  uint8_t has_flush = 0;
  uint8_t has_straight = 0;
  uint8_t has_ace = 0;

  Rank highest_card = RANK_TWO, lowest_card = RANK_KING;

  for (uint8_t i = 0; i < cvector_size(hand->cards); i++) {
    Card card = hand->cards[i];
    if (card.selected == 0) {
      continue;
    }

    if (card.rank == RANK_ACE) {
      has_ace = 1;
    }
    if (card.rank != RANK_ACE && card.rank > highest_card) {
      highest_card = card.rank;
    }
    if (card.rank != RANK_ACE && card.rank < lowest_card) {
      lowest_card = card.rank;
    }

    rank_counts[card.rank] += 1;
    suit_counts[card.suit] += 1;

    if (rank_counts[card.rank] > 1) {
      // this means there are duplicates in selected ranks,
      // so straight is not possible
      has_straight = 2;
      x_of_kind[rank_counts[card.rank] - 2] += 1;
      if (rank_counts[card.rank] > 2) {
        x_of_kind[rank_counts[card.rank] - 3] -= 1;
      }
    }

    if (suit_counts[card.suit] == 5) {
      has_flush = 1;
    }
  }

  // A 2 3 4 5 is also valid straight, so it needs to be checked
  if (has_straight == 0 && selected_count == 5 &&
      ((has_ace == 0 && highest_card - lowest_card == 4) ||
       (has_ace == 1 && (13 - lowest_card == 4 || highest_card == 4)))) {
    has_straight = 1;
  }

  if (has_flush == 1 && x_of_kind[5 - 2] > 0) {
    return HAND_FLUSH_FIVE;
  }

  if (has_flush == 1 && x_of_kind[3 - 2] == 1 && x_of_kind[2 - 2] == 1) {
    return HAND_FLUSH_HOUSE;
  }

  if (x_of_kind[5 - 2] == 1) {
    return HAND_FIVE_OF_KIND;
  }

  if (has_flush == 1 && has_straight == 1) {
    return HAND_STRAIGHT_FLUSH;
  }

  if (x_of_kind[4 - 2] == 1) {
    return HAND_FOUR_OF_KIND;
  }

  if (x_of_kind[3 - 2] == 1 && x_of_kind[2 - 2] == 1) {
    return HAND_FULL_HOUSE;
  }

  if (has_flush == 1) {
    return HAND_FLUSH;
  }

  if (has_straight == 1) {
    return HAND_STRAIGHT;
  }

  if (x_of_kind[3 - 2] == 1) {
    return HAND_THREE_OF_KIND;
  }

  if (x_of_kind[2 - 2] >= 2) {
    return HAND_TWO_PAIR;
  }

  if (x_of_kind[2 - 2] >= 1) {
    return HAND_PAIR;
  }

  return HAND_HIGH_CARD;
}

void update_scoring_hand() {
  PokerHand poker_hand = evaluate_hand();

  state.game.selected_hand.poker_hand = poker_hand;
  const Hand *hand = &state.game.hand;
  Card *selected_cards[5] = {};
  Card **scoring_cards = state.game.selected_hand.scoring_cards;

  // Clear previous scoring cards
  memset(scoring_cards, 0, 5 * sizeof(Card *));

  uint8_t j = 0;
  for (uint8_t i = 0; i < cvector_size(hand->cards); i++) {
    if (hand->cards[i].selected == 0) {
      continue;
    }

    selected_cards[j] = &hand->cards[i];
    j++;
  }

  if (selected_cards[0] == NULL) {
    return;
  }

  // All of those hands require 5 selected cards
  if (poker_hand == HAND_FLUSH_FIVE || poker_hand == HAND_FLUSH_HOUSE ||
      poker_hand == HAND_FIVE_OF_KIND || poker_hand == HAND_STRAIGHT_FLUSH ||
      poker_hand == HAND_FULL_HOUSE || poker_hand == HAND_FLUSH ||
      poker_hand == HAND_STRAIGHT) {
    for (uint8_t i = 0; i < 5; i++) {
      if (selected_cards[i] == NULL) {
        return;
      }

      scoring_cards[i] = selected_cards[i];
    }
  }

  uint8_t rank_counts[13] = {};
  uint8_t highest_card_index = 0;
  Rank scoring_rank = selected_cards[0]->rank;
  uint8_t scoring_count = 0;
  for (uint8_t i = 0; i < 5; i++) {
    if (selected_cards[i] == NULL) {
      break;
    }

    if (selected_cards[highest_card_index]->rank != RANK_ACE &&
        (selected_cards[i]->rank == RANK_ACE ||
         selected_cards[i]->rank > selected_cards[highest_card_index]->rank)) {
      highest_card_index = i;
    }

    rank_counts[selected_cards[i]->rank]++;
    if (rank_counts[selected_cards[i]->rank] > scoring_count) {
      scoring_rank = selected_cards[i]->rank;
      scoring_count = rank_counts[scoring_rank];
    }
  }

  if (poker_hand == HAND_FOUR_OF_KIND || poker_hand == HAND_THREE_OF_KIND ||
      poker_hand == HAND_PAIR || poker_hand == HAND_TWO_PAIR) {
    uint8_t j = 0;

    Rank second_scoring_rank = scoring_rank;
    if (poker_hand == HAND_TWO_PAIR) {
      for (uint8_t i = 0; i < 13; i++) {
        if (rank_counts[i] == 2 && i != scoring_rank) {
          second_scoring_rank = i;
          break;
        }
      }
    }

    for (uint8_t i = 0; i < 5; i++) {
      if (selected_cards[i] == NULL ||
          (selected_cards[i]->rank != scoring_rank &&
           selected_cards[i]->rank != second_scoring_rank)) {
        continue;
      }

      scoring_cards[j] = selected_cards[i];
      j++;
    }
  }

  if (poker_hand == HAND_HIGH_CARD) {
    scoring_cards[0] = selected_cards[highest_card_index];
  }

  PokerHandScoring scoring = get_poker_hand_base_scoring(poker_hand);
  state.game.selected_hand.chips = scoring.chips;
  state.game.selected_hand.mult = scoring.mult;
  for (uint8_t i = 0; i < 5; i++) {
    if (scoring_cards[i] == NULL) {
      continue;
    }

    state.game.selected_hand.chips += scoring_cards[i]->chips;
  }
}

char *get_poker_hand_name(PokerHand hand) {
  switch (hand) {
  case HAND_FLUSH_FIVE:
    return "Flush Five";
  case HAND_FLUSH_HOUSE:
    return "Flush House";
  case HAND_FIVE_OF_KIND:
    return "Five of Kind";
  case HAND_STRAIGHT_FLUSH:
    return "Straight Flush";
  case HAND_FOUR_OF_KIND:
    return "Four of Kind";
  case HAND_FULL_HOUSE:
    return "Full House";
  case HAND_FLUSH:
    return "Flush";
  case HAND_STRAIGHT:
    return "Straight";
  case HAND_THREE_OF_KIND:
    return "Three of Kind";
  case HAND_TWO_PAIR:
    return "Two Pair";
  case HAND_PAIR:
    return "Pair";
  case HAND_HIGH_CARD:
    return "High Card";
  }

  return "UNDEFINED HAND";
}

PokerHandScoring get_poker_hand_base_scoring(PokerHand hand) {
  switch (hand) {
  case HAND_FLUSH_FIVE:
    return (PokerHandScoring){.mult = 16, .chips = 160};
  case HAND_FLUSH_HOUSE:
    return (PokerHandScoring){.mult = 14, .chips = 140};
  case HAND_FIVE_OF_KIND:
    return (PokerHandScoring){.mult = 12, .chips = 120};
  case HAND_STRAIGHT_FLUSH:
    return (PokerHandScoring){.mult = 8, .chips = 100};
  case HAND_FOUR_OF_KIND:
    return (PokerHandScoring){.mult = 7, .chips = 60};
  case HAND_FULL_HOUSE:
    return (PokerHandScoring){.mult = 4, .chips = 40};
  case HAND_FLUSH:
    return (PokerHandScoring){.mult = 4, .chips = 35};
  case HAND_STRAIGHT:
    return (PokerHandScoring){.mult = 4, .chips = 30};
  case HAND_THREE_OF_KIND:
    return (PokerHandScoring){.mult = 3, .chips = 30};
  case HAND_TWO_PAIR:
    return (PokerHandScoring){.mult = 2, .chips = 20};
  case HAND_PAIR:
    return (PokerHandScoring){.mult = 2, .chips = 10};
  case HAND_HIGH_CARD:
    return (PokerHandScoring){.mult = 1, .chips = 5};
  }

  return (PokerHandScoring){.mult = 0, .chips = 0};
}

double get_ante_base_score(uint8_t ante) {
  switch (ante) {
  case 0:
    return 100;
  case 1:
    return 300;
  case 2:
    return 800;
  case 3:
    return 2000;
  case 4:
    return 5000;
  case 5:
    return 11000;
  case 6:
    return 20000;
  case 7:
    return 35000;
  case 8:
    return 50000;
  }

  return 0;
}

double get_required_score(uint8_t ante, uint8_t blind) {
  return get_ante_base_score(ante) * (blind == 0 ? 1 : blind == 1 ? 1.5 : 2);
}

uint8_t get_blind_money(uint8_t blind) {
  return blind == 0 ? 3 : blind == 1 ? 4 : 5;
}

void buy_shop_item() {
  uint8_t shopCount = cvector_size(state.game.shop.items);
  ShopItem item = state.game.shop.items[state.game.shop.selected_card];

  if (state.game.shop.selected_card >= shopCount ||
      state.game.money < item.price)
    return;

  switch (item.type) {
  case SHOP_ITEM_JOKER:
    if (cvector_size(state.game.jokers.cards) >= state.game.jokers.size)
      return;

    cvector_push_back(state.game.jokers.cards, item.joker);
    break;

  case SHOP_ITEM_CARD:
    cvector_push_back(state.game.full_deck.cards, item.card);
    break;
  }

  state.game.money -= item.price;
  cvector_erase(state.game.shop.items, state.game.shop.selected_card);
  shopCount--;

  if (state.game.shop.selected_card >= shopCount) {
    state.game.shop.selected_card = shopCount - 1;
  }
}

void exit_shop() {
  state.game.round++;
  state.game.blind++;

  if (state.game.blind > 2) {
    state.game.blind = 0;
    state.game.ante++;
  }

  state.game.score = 0;

  // Reset hand and deck for new blind
  state.game.hands = 4;
  state.game.discards = 2;
  cvector_clear(state.game.hand.cards);
  cvector_copy(state.game.full_deck.cards, state.game.deck.cards);
  shuffle_deck();

  fill_hand();

  state.stage = STAGE_GAME;
}
