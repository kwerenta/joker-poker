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

typedef enum {
  // Secret planets
  PLANET_ERIS,
  PLANET_CERES,
  PLANET_X,

  PLANET_NEPTUNE,
  PLANET_MARS,
  PLANET_EARTH,
  PLANET_JUPITER,
  PLANET_SATURN,
  PLANET_VENUS,
  PLANET_URANUS,
  PLANET_MERCURY,
  PLANET_PLUTO,
} Planet;

typedef struct {
  uint32_t mult;
  uint32_t chips;
} PokerHandScoring;

typedef enum {
  RARITY_COMMON,
  RARITY_UNCOMMON,
  RARITY_RARE,
  RARITY_LEGENDARY
} Rarity;

typedef enum {
  ACTIVATION_INDEPENDENT,
  ACTIVATION_ON_SCORED,
  ACTIVATION_ON_HELD
} ActivationType;

typedef struct {
  uint16_t id;
  const char *name;
  const char *description;
  uint8_t base_price;
  Rarity rarity;
  ActivationType activation_type;
  void (*activate)();
} Joker;

extern const Joker JOKERS[];
extern const uint8_t JOKER_COUNT;

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
} Hand;

typedef struct {
  uint8_t size;
  cvector_vector_type(Joker) cards;
} JokerHand;

typedef struct {
  uint8_t count;
  PokerHand poker_hand;
  PokerHandScoring scoring;
  Card *scoring_cards[5];
} SelectedHand;

typedef enum {
  SHOP_ITEM_JOKER,
  SHOP_ITEM_CARD,
  SHOP_ITEM_PLANET,
  SHOP_ITEM_BOOSTER_PACK
} ShopItemType;

typedef enum {
  BOOSTER_PACK_BUFFON,
  BOOSTER_PACK_CELESTIAL,
  BOOSTER_PACK_STANDARD
} BoosterPackType;

typedef enum {
  BOOSTER_PACK_NORMAL,
  BOOSTER_PACK_JUMBO,
  BOOSTER_PACK_MEGA,
} BoosterPackSize;

typedef struct {
  BoosterPackType type;
  BoosterPackSize size;
} BoosterPackItem;

typedef struct {
  uint8_t selected;
  union {
    Card card;
    Joker joker;
    Planet planet;
  };
} BoosterPackContent;

typedef struct {
  BoosterPackItem item;
  cvector_vector_type(BoosterPackContent) content;
  uint8_t hovered_item;
} BoosterPack;

typedef struct {
  ShopItemType type;
  union {
    Joker joker;
    Card card;
    Planet planet;
    BoosterPackItem booster_pack;
  };
} ShopItem;

typedef struct {
  cvector_vector_type(ShopItem) items;
  uint8_t selected_card;
} Shop;

typedef struct {
  cvector_vector_type(Card) full_deck;
  cvector_vector_type(Card) deck;
  Hand hand;
  SelectedHand selected_hand;
  JokerHand jokers;

  double score;
  uint8_t ante;
  uint8_t round;
  uint8_t blind;

  uint8_t hands;
  uint8_t discards;

  uint8_t poker_hands[12];

  uint16_t money;
  Shop shop;
  BoosterPack booster_pack;
} Game;

void game_init();
void game_destroy();
Card create_card(Suit suit, Rank rank);

void shuffle_deck();
void draw_card();
void play_hand();
void discard_hand();
void fill_hand();
void sort_hand(uint8_t by_suit);

void set_hovered_card(uint8_t *hovered, uint8_t new_position);
void toggle_card_select(uint8_t index);
void move_card_in_hand(uint8_t *hovered, uint8_t new_position);
void deselect_all_cards();
void remove_selected_cards();

PokerHand evaluate_hand();
void update_scoring_hand();

PokerHandScoring get_poker_hand_base_scoring(PokerHand hand);
PokerHandScoring get_planet_card_base_scoring(PokerHand hand);
PokerHandScoring get_poker_hand_total_scoring(PokerHand hand);
double get_ante_base_score(uint8_t ante);
double get_required_score(uint8_t ante, uint8_t blind);
uint8_t get_blind_money(uint8_t blind);

uint8_t get_shop_item_price(ShopItem *item);
void buy_shop_item();
void open_booster_pack(BoosterPackItem booster_pack);
void submit_booster_pack();
void toggle_booster_pack_item_select();
void restock_shop();
void exit_shop();

#endif
