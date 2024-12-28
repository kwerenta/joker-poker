#include "game.h"
#include "state.h"

void game_init() {
  state.game.deck = (Deck){
      .size = 52, .cards = (Card *)malloc(52 * sizeof(Card)), .count = 0};
  for (uint8_t i = 0; i < 52; i++) {
    state.game.deck.cards[state.game.deck.count] =
        (Card){.suit = i % 4, .rank = i % 13, .selected = 0};
    state.game.deck.count++;
  }

  state.game.hand =
      (Hand){.size = 7, .cards = (Card *)malloc(7 * sizeof(Card)), .count = 0};
  for (uint8_t i = 0; i < state.game.hand.size; i++) {
    state.game.hand.cards[state.game.hand.count] =
        state.game.deck.cards[rand() % 52];
    state.game.hand.count++;
  }
}

void game_destroy() {
  free(state.game.deck.cards);
  free(state.game.hand.cards);
}

void set_hovered_card(uint8_t *hovered, uint8_t new_position) {
  if (new_position >= state.game.hand.count) {
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
  for (uint8_t i = 0; i < hand->count; i++) {
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

  if (new_position >= hand->count) {
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

  for (uint8_t i = 0; i < hand->count; i++) {
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
  for (uint8_t i = 0; i < hand->count; i++) {
    if (hand->cards[i].selected == 0) {
      continue;
    }

    selected_cards[j] = &hand->cards[i];
    j++;
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
