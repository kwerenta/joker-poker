#ifndef GAME_H
#define GAME_H

#include "lib/cvector.h"
#include <stdint.h>

typedef enum { SUIT_HEARTS, SUIT_DIAMONDS, SUIT_SPADES, SUIT_CLUBS } Suit;
typedef enum {
  RANK_ACE,
  RANK_TWO,
  RANK_THREE,
  RANK_FOUR,
  RANK_FIVE,
  RANK_SIX,
  RANK_SEVEN,
  RANK_EIGHT,
  RANK_NINE,
  RANK_TEN,
  RANK_JACK,
  RANK_QUEEN,
  RANK_KING
} Rank;

typedef enum {
  HAND_FLUSH_FIVE,
  HAND_FLUSH_HOUSE,
  HAND_FIVE_OF_KIND,
  HAND_STRAIGHT_FLUSH,
  HAND_FOUR_OF_KIND,
  HAND_FULL_HOUSE,
  HAND_FLUSH,
  HAND_STRAIGHT,
  HAND_THREE_OF_KIND,
  HAND_TWO_PAIR,
  HAND_PAIR,
  HAND_HIGH_CARD
} PokerHand;

typedef struct {
  uint64_t mult;
  uint64_t chips;
} PokerHandScoring;

typedef struct {
  Suit suit;
  Rank rank;
  uint8_t selected;
  uint16_t chips;
} Card;

typedef struct {
  // Max number of cards in structure that can be obtained naturally
  // (some bosses/jokers will be able to overflow this value)
  uint8_t size;
  cvector_vector_type(Card) cards;
} Hand, Deck;

typedef struct {
  uint8_t count;
  PokerHand poker_hand;
  uint64_t chips;
  uint64_t mult;
  Card *scoring_cards[5];
} SelectedHand;

typedef struct {
  Deck full_deck;
  Deck deck;
  Hand hand;
  SelectedHand selected_hand;

  double score;
  uint8_t ante;
  uint8_t round;
  uint8_t blind;
} Game;

void game_init();
void game_destroy();
Card create_card(Suit suit, Rank rank);

void shuffle_deck();
void draw_card();
void play_hand();
void discard_hand();
void fill_hand();

void set_hovered_card(uint8_t *hovered, uint8_t new_position);
void toggle_card_select(uint8_t index);
void move_card_in_hand(uint8_t *hovered, uint8_t new_position);
void deselect_all_cards();
void remove_selected_cards();

PokerHand evaluate_hand();
void update_scoring_hand();

PokerHandScoring get_poker_hand_base_scoring(PokerHand hand);
double get_ante_base_score(uint8_t ante);

char *get_poker_hand_name(PokerHand hand);

#endif
