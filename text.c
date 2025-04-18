#include "text.h"

#include "game.h"
#include "state.h"

char *get_poker_hand_name(uint16_t hand_union) {
  switch (get_poker_hand(hand_union)) {
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
}

char *get_planet_card_name(Planet planet) {
  switch (planet) {
    case PLANET_ERIS:
      return "Eris";
    case PLANET_CERES:
      return "Ceres";
    case PLANET_X:
      return "Planet X";
    case PLANET_NEPTUNE:
      return "Neptune";
    case PLANET_MARS:
      return "Mars";
    case PLANET_EARTH:
      return "Earth";
    case PLANET_JUPITER:
      return "Jupiter";
    case PLANET_SATURN:
      return "Saturn";
    case PLANET_VENUS:
      return "Venus";
    case PLANET_URANUS:
      return "Uranus";
    case PLANET_MERCURY:
      return "Mercury";
    case PLANET_PLUTO:
      return "Pluto";
  }
}

char *get_card_suit_name(Suit suit) {
  switch (suit) {
    case SUIT_HEARTS:
      return "Hearts";
    case SUIT_DIAMONDS:
      return "Diamonds";
    case SUIT_SPADES:
      return "Spades";
    case SUIT_CLUBS:
      return "Clubs";
  }
}

char *get_card_rank_name(Rank rank) {
  switch (rank) {
    case RANK_ACE:
      return "Ace";
    case RANK_TWO:
      return "2";
    case RANK_THREE:
      return "3";
    case RANK_FOUR:
      return "4";
    case RANK_FIVE:
      return "5";
    case RANK_SIX:
      return "6";
    case RANK_SEVEN:
      return "7";
    case RANK_EIGHT:
      return "8";
    case RANK_NINE:
      return "9";
    case RANK_TEN:
      return "10";
    case RANK_JACK:
      return "Jack";
    case RANK_QUEEN:
      return "Queen";
    case RANK_KING:
      return "King";
  }
}

Clay_String get_full_card_name(Suit suit, Rank rank) {
  Clay_String card_name;
  append_clay_string(&card_name, "%s of %s", get_card_rank_name(rank), get_card_suit_name(suit));

  return card_name;
}

char *get_booster_pack_size_name(BoosterPackSize size) {
  switch (size) {
    case BOOSTER_PACK_NORMAL:
      return "";
    case BOOSTER_PACK_JUMBO:
      return "Jumbo";
    case BOOSTER_PACK_MEGA:
      return "Mega";
  }
}

char *get_booster_pack_type_name(BoosterPackType type) {
  switch (type) {
    case BOOSTER_PACK_STANDARD:
      return "Standard";
    case BOOSTER_PACK_BUFFON:
      return "Buffon";
    case BOOSTER_PACK_CELESTIAL:
      return "Celestial";
    case BOOSTER_PACK_ARCANA:
      return "Arcana";
  }
}

Clay_String get_full_booster_pack_name(BoosterPackSize size, BoosterPackType type) {
  Clay_String booster_pack_name;
  append_clay_string(&booster_pack_name, "%s%s%s Pack", get_booster_pack_size_name(size),
                     size == BOOSTER_PACK_NORMAL ? "" : " ", get_booster_pack_type_name(type));

  return booster_pack_name;
}
