#include "game.h"
#include "lib/cvector.h"
#include "state.h"

void game_init() {
  // Generate standard deck of 52 cards
  cvector_reserve(state.game.deck.cards, 52);
  for (uint8_t i = 0; i < cvector_capacity(state.game.deck.cards); i++) {
    Card card = {.suit = i % 4, .rank = i % 13, .selected = 0};
    cvector_push_back(state.game.deck.cards, card);
  }

  shuffle_deck();

  cvector_reserve(state.game.hand.cards, 7);
  for (uint8_t i = 0; i < cvector_capacity(state.game.hand.cards); i++) {
    draw_card();
  }
}

void game_destroy() {
  cvector_free(state.game.deck.cards);
  cvector_free(state.game.hand.cards);
}

void draw_card() {
  cvector_push_back(
      state.game.hand.cards,
      state.game.deck.cards[cvector_size(state.game.deck.cards) - 1]);
  cvector_pop_back(state.game.deck.cards);
}

void play_hand() {
  get_scoring_hand();

  uint8_t i = 0;
  while (i < cvector_size(state.game.hand.cards)) {
    if (state.game.hand.cards[i].selected == 1) {
      cvector_erase(state.game.hand.cards, i);
      continue;
    }

    i++;
  }

  while (cvector_size(state.game.hand.cards) <
         cvector_capacity(state.game.hand.cards)) {
    draw_card();
  }
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

void toggle_card_select(uint8_t hovered) {
  Hand *hand = &state.game.hand;

  if (hand->cards[hovered].selected == 1) {
    hand->cards[hovered].selected = 0;
    return;
  }

  uint8_t selected_count = 0;
  for (uint8_t i = 0; i < cvector_size(hand->cards); i++) {
    if (hand->cards[i].selected == 0) {
      continue;
    }

    selected_count++;

    if (selected_count == 5) {
      return;
    }
  }

  hand->cards[hovered].selected = 1;
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

PokerHand evaluate_hand() {
  const Hand *hand = &state.game.hand;

  uint8_t rank_counts[13] = {};
  uint8_t suit_counts[4] = {};
  uint8_t selected_count = 0;

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

    selected_count += 1;

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

void get_scoring_hand() {
  PokerHand poker_hand = evaluate_hand();

  const Hand *hand = &state.game.hand;
  Card *selected_cards[5] = {};
  Card *scoring_cards[5] = {};

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
        break;
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

  printf("Scoring hand: \n");
  for (uint8_t i = 0; i < 5; i++) {
    if (scoring_cards[i] == NULL) {
      break;
    }

    printf("\t- %d of %d\n", scoring_cards[i]->rank, scoring_cards[i]->suit);
  }
  printf("\n");
}
