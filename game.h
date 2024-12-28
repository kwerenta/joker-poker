#ifndef GAME_H
#define GAME_H

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
  Suit suit;
  Rank rank;
  uint8_t selected;
} Card;

typedef struct {
  uint8_t size;
  uint8_t count;
  Card *cards;
} Hand, Deck;

typedef struct {
  Deck deck;
  Hand hand;
} Game;

void game_init();

void set_hovered_card(uint8_t *hovered, uint8_t new_position);
void toggle_card_select(uint8_t hovered);
void move_card_in_hand(uint8_t *hovered, uint8_t new_position);
PokerHand evaluate_hand();
void get_scoring_hand();

#endif
