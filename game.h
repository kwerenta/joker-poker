#ifndef GAME_H
#define GAME_H

#include <cvector.h>
#include <stdint.h>

#include "content/spectral.h"
#include "content/tarot.h"

typedef enum {
  DECK_RED,
  DECK_BLUE,
  DECK_YELLOW,
  DECK_GREEN,
  DECK_BLACK,
  DECK_MAGIC,
  DECK_NEBULA,
  DECK_GHOST,
  DECK_ABANDONED,
  DECK_CHECKERED,
  DECK_ZODIAC,
  DECK_PAINTED,
  DECK_ANAGLYPH,
  DECK_PLASMA,
  DECK_ERRATIC,
} Deck;

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

typedef enum { SEAL_NONE, SEAL_GOLD, SEAL_RED, SEAL_BLUE, SEAL_PURPLE } Seal;

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
  uint8_t level;
  uint16_t played;
} PokerHandStats;

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
  Seal seal;

  uint16_t chips;
  uint8_t selected;
} Card;

typedef struct {
  // Max number of cards in structure that can be obtained naturally
  // (some bosses/jokers will be able to overflow this value)
  uint8_t size;
  cvector_vector_type(Card) cards;
} Hand;

typedef enum { CONSUMABLE_PLANET, CONSUMABLE_TAROT, CONSUMABLE_SPECTRAL } ConsumableType;

typedef struct {
  ConsumableType type;

  union {
    Planet planet;
    Tarot tarot;
    Spectral spectral;
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

typedef enum { SHOP_ITEM_JOKER, SHOP_ITEM_CARD, SHOP_ITEM_PLANET, SHOP_ITEM_TAROT, SHOP_ITEM_SPECTRAL } ShopItemType;

typedef enum {
  BOOSTER_PACK_BUFFON,
  BOOSTER_PACK_STANDARD,
  BOOSTER_PACK_CELESTIAL,
  BOOSTER_PACK_ARCANA,
  BOOSTER_PACK_SPECTRAL
} BoosterPackType;

typedef enum { BOOSTER_PACK_NORMAL, BOOSTER_PACK_JUMBO, BOOSTER_PACK_MEGA } BoosterPackSize;

typedef struct {
  BoosterPackType type;
  BoosterPackSize size;
} BoosterPackItem;

typedef union {
  Card card;
  Joker joker;
  Planet planet;
  Tarot tarot;
  Spectral spectral;
} BoosterPackContent;

typedef struct {
  uint8_t uses;
  BoosterPackItem item;
  cvector_vector_type(BoosterPackContent) content;
} BoosterPack;

typedef enum {
  // Base vouchers
  VOUCHER_OVERSTOCK = 1 << 0,
  VOUCHER_CLEARANCE_SALE = 1 << 1,
  VOUCHER_HONE = 1 << 2,
  VOUCHER_REROLL_SURPLUS = 1 << 3,
  VOUCHER_CRYSTALL_BALL = 1 << 4,
  VOUCHER_TELESCOPE = 1 << 5,
  VOUCHER_GRABBER = 1 << 6,
  VOUCHER_WASTEFUL = 1 << 7,
  VOUCHER_TAROT_MERCHANT = 1 << 8,
  VOUCHER_PLANET_MERCHANT = 1 << 9,
  VOUCHER_SEED_MONEY = 1 << 10,
  VOUCHER_BLANK = 1 << 11,
  VOUCHER_MAGIC_TRICK = 1 << 12,
  VOUCHER_HIEROGLYPH = 1 << 13,
  VOUCHER_DIRECTORS_CUT = 1 << 14,
  VOUCHER_PAINT_BRUSH = 1 << 15,

  // Upgraded vouchers
  VOUCHER_OVERSTOCK_PLUS = 1 << (0 + 16),
  VOUCHER_LIQUIDATION = 1 << (1 + 16),
  VOUCHER_GLOW_UP = 1 << (2 + 16),
  VOUCHER_REROLL_GLUT = 1 << (3 + 16),
  VOUCHER_OMEN_GLOBE = 1 << (4 + 16),
  VOUCHER_OBSERVATORY = 1 << (5 + 16),
  VOUCHER_NACHO_TONG = 1 << (6 + 16),
  VOUCHER_RECYCLOMANCY = 1 << (7 + 16),
  VOUCHER_TAROT_TYCOON = 1 << (8 + 16),
  VOUCHER_PLANET_TYCOON = 1 << (9 + 16),
  VOUCHER_MONEY_TREE = 1 << (10 + 16),
  VOUCHER_ANTIMATTER = 1 << (11 + 16),
  VOUCHER_ILLUSION = 1 << (12 + 16),
  VOUCHER_PTEROGLYPH = 1 << (13 + 16),
  VOUCHER_RETCON = 1 << (14 + 16),
  VOUCHER_PALETTE = 1 << (15 + 16)
} Voucher;

typedef struct {
  ShopItemType type;
  union {
    Joker joker;
    Card card;
    Planet planet;
    Tarot tarot;
    Spectral spectral;
  };
} ShopItem;

typedef struct {
  uint8_t size;
  Voucher voucher;
  cvector_vector_type(ShopItem) items;
  cvector_vector_type(BoosterPackItem) booster_packs;
} Shop;

typedef enum { SORTING_BY_RANK, SORTING_BY_SUIT } SortingMode;

typedef struct {
  uint8_t remaining;
  uint8_t total;
} UsageState;

typedef struct {
  Deck deck_type;
  cvector_vector_type(Card) full_deck;
  cvector_vector_type(Card) deck;

  Hand hand;
  SelectedHand selected_hand;

  uint32_t vouchers;
  JokerHand jokers;
  Consumables consumables;

  double score;
  uint8_t ante;
  uint8_t round;
  uint8_t blind;

  UsageState hands;
  UsageState discards;

  PokerHandStats poker_hands[12];

  uint16_t money;
  Shop shop;

  BoosterPack booster_pack;
  FoolLastUsed fool_last_used;

  SortingMode sorting_mode;
} Game;

void game_init(Deck deck);
void game_destroy();

uint8_t compare_cards(Card *a, Card *b);
Card create_card(Suit suit, Rank rank, Edition edition, Enhancement enchacement, Seal seal);
void shuffle_deck();
void draw_card();
void play_hand();
void discard_hand();
void fill_hand();
void sort_hand();

void toggle_card_select(uint8_t index);
void deselect_all_cards();
void remove_selected_cards();

uint16_t evaluate_hand();
uint8_t does_poker_hand_contain(uint16_t hand_union, PokerHand expected);
PokerHand get_poker_hand(uint16_t hand_union);
void update_scoring_hand();
void update_scoring_edition(Edition edition);

PokerHandStats *get_poker_hand_stats(uint16_t hand_union);
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
uint8_t add_item_to_player(ShopItem *item);
uint8_t get_shop_item_price(ShopItem *item);
uint8_t get_voucher_price(Voucher voucher);
void add_voucher_to_player(Voucher voucher);
uint8_t get_booster_pack_price(BoosterPackItem *booster_pack);
uint8_t get_booster_pack_items_count(BoosterPackItem *booster_pack);
uint8_t get_shop_item_sell_price(ShopItem *item);
void buy_shop_item();
void sell_shop_item();
void open_booster_pack(BoosterPackItem *booster_pack);
void select_booster_pack_item();
void skip_booster_pack();
void fill_shop_items();
void restock_shop();
void exit_shop();

#endif
