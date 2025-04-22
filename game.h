#ifndef GAME_H
#define GAME_H

#include <cvector.h>
#include <stdint.h>

#include "content/tarot.h"

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

typedef enum { EDITION_BASE, EDITION_FOIL, EDITION_HOLOGRAPHIC, EDITION_POLYCHROME, EDITION_NEGATIVE } Edition;

typedef enum {
  ENHANCEMENT_NONE,
  ENHANCEMENT_BONUS,
  ENHANCEMENT_MULT,
  ENHANCEMENT_WILD,
  ENHANCEMENT_GLASS,
  ENHANCEMENT_STEEL,
  ENHANCEMENT_STONE,
  ENHANCEMENT_GOLD,
  ENHANCEMENT_LUCKY
} Enhancement;

typedef enum {
  HAND_FLUSH_FIVE = 1 << 0,
  HAND_FLUSH_HOUSE = 1 << 1,
  HAND_FIVE_OF_KIND = 1 << 2,

  HAND_STRAIGHT_FLUSH = 1 << 3,
  HAND_FOUR_OF_KIND = 1 << 4,
  HAND_FULL_HOUSE = 1 << 5,
  HAND_FLUSH = 1 << 6,
  HAND_STRAIGHT = 1 << 7,
  HAND_THREE_OF_KIND = 1 << 8,
  HAND_TWO_PAIR = 1 << 9,
  HAND_PAIR = 1 << 10,
  HAND_HIGH_CARD = 1 << 11
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
  double mult;
  uint32_t chips;
} ScorePair;

typedef enum { RARITY_COMMON, RARITY_UNCOMMON, RARITY_RARE, RARITY_LEGENDARY } Rarity;

typedef enum { ACTIVATION_INDEPENDENT, ACTIVATION_ON_SCORED, ACTIVATION_ON_HELD } ActivationType;

typedef struct {
  uint16_t id;
  const char *name;
  const char *description;
  uint8_t base_price;

  Rarity rarity;
  Edition edition;

  ActivationType activation_type;
  void (*activate)();
} Joker;

extern const Joker JOKERS[];
extern const uint8_t JOKER_COUNT;

typedef struct {
  Suit suit;
  Rank rank;

  Edition edition;
  Enhancement enhancement;

  uint16_t chips;
  uint8_t selected;
} Card;

typedef struct {
  // Max number of cards in structure that can be obtained naturally
  // (some bosses/jokers will be able to overflow this value)
  uint8_t size;
  cvector_vector_type(Card) cards;
} Hand;

typedef enum { CONSUMABLE_PLANET, CONSUMABLE_TAROT } ConsumableType;

typedef struct {
  ConsumableType type;

  union {
    Planet planet;
    Tarot tarot;
  };
} Consumable;

typedef struct {
  uint8_t was_used;
  Consumable consumable;
} FoolLastUsed;

typedef struct {
  int8_t hovered;
  uint8_t size;
  cvector_vector_type(Consumable) items;
} Consumables;

typedef struct {
  uint8_t size;
  cvector_vector_type(Joker) cards;
} JokerHand;

typedef struct {
  uint8_t count;
  uint16_t hand_union;
  ScorePair score_pair;
  Card *scoring_cards[5];
} SelectedHand;

typedef enum { SHOP_ITEM_JOKER, SHOP_ITEM_CARD, SHOP_ITEM_PLANET, SHOP_ITEM_TAROT } ShopItemType;

typedef enum {
  BOOSTER_PACK_BUFFON,
  BOOSTER_PACK_STANDARD,
  BOOSTER_PACK_CELESTIAL,
  BOOSTER_PACK_ARCANA
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

typedef union {
  Card card;
  Joker joker;
  Planet planet;
  Tarot tarot;
} BoosterPackContent;

typedef struct {
  uint8_t uses;
  BoosterPackItem item;
  cvector_vector_type(BoosterPackContent) content;
} BoosterPack;

typedef struct {
  ShopItemType type;
  union {
    Joker joker;
    Card card;
    Planet planet;
    Tarot tarot;
  };
} ShopItem;

typedef struct {
  uint8_t size;
  cvector_vector_type(ShopItem) items;
  cvector_vector_type(BoosterPackItem) booster_packs;
} Shop;

typedef enum { SORTING_BY_SUIT, SORTING_BY_RANK } SortingMode;

typedef struct {
  cvector_vector_type(Card) full_deck;
  cvector_vector_type(Card) deck;

  Hand hand;
  SelectedHand selected_hand;

  JokerHand jokers;
  Consumables consumables;

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
  FoolLastUsed fool_last_used;

  SortingMode sorting_mode;
} Game;

void game_init();
void game_destroy();

uint8_t compare_cards(Card *a, Card *b);
Card create_card(Suit suit, Rank rank, Edition edition, Enhancement enchacement);
void shuffle_deck();
void draw_card();
void play_hand();
void discard_hand();
void fill_hand();
void sort_hand(SortingMode sorting_mode);

void toggle_card_select(uint8_t index);
void deselect_all_cards();
void remove_selected_cards();

uint16_t evaluate_hand();
uint8_t does_poker_hand_contain(uint16_t hand_union, PokerHand expected);
PokerHand get_poker_hand(uint16_t hand_union);
void update_scoring_hand();
void update_scoring_edition(Edition edition);

ScorePair get_poker_hand_base_score(uint16_t hand_union);
ScorePair get_planet_card_base_score(uint16_t hand_union);
ScorePair get_poker_hand_total_score(uint16_t hand_union);
double get_ante_base_score(uint8_t ante);
double get_required_score(uint8_t ante, uint8_t blind);

uint8_t get_blind_money(uint8_t blind);
uint8_t get_hands_money();
uint8_t get_interest_money();

void get_cash_out();

uint8_t use_consumable(Consumable *consumable);
uint8_t get_shop_item_price(ShopItem *item);
uint8_t get_booster_pack_price(BoosterPackItem *booster_pack);
uint8_t get_booster_pack_items_count(BoosterPackItem *booster_pack);
void buy_shop_item();
void open_booster_pack(BoosterPackItem *booster_pack);
void select_booster_pack_item();
void skip_booster_pack();
void restock_shop();
void exit_shop();

#endif
