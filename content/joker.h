#ifndef JOKER_H
#define JOKER_H

#include <stdint.h>

struct Card;

typedef enum {
  JOKER_JOKER = 1,
  JOKER_JOLLY = 6,
} JokerId;

typedef enum { EDITION_BASE, EDITION_FOIL, EDITION_HOLOGRAPHIC, EDITION_POLYCHROME, EDITION_NEGATIVE } Edition;

typedef enum { RARITY_COMMON, RARITY_UNCOMMON, RARITY_RARE, RARITY_LEGENDARY } Rarity;

typedef enum { CARD_STATUS_NORMAL = 0, CARD_STATUS_FACE_DOWN = 1 << 0, CARD_STATUS_DEBUFFED = 1 << 1 } CardStatus;

typedef struct Joker {
  JokerId id;
  const char *name;
  const char *description;
  uint8_t base_price;

  Rarity rarity;
  Edition edition;

  void (*activate_on_played)(struct Joker *self);
  void (*activate_on_scored)(struct Joker *self, struct Card *card);
  void (*activate_on_held)(struct Joker *self, struct Card *card);
  void (*activate_independent)(struct Joker *self);
  void (*activate_on_other_jokers)(struct Joker *self, struct Joker *other);
  void (*activate_on_discard)(struct Joker *self, struct Card *card);
  void (*activate_on_blind_select)(struct Joker *self);
  void (*activate_passive)(struct Joker *self);

  void (*scale_on_played)(struct Joker *self);
  void (*scale_on_scored)(struct Joker *self, struct Card *card);
  void (*scale_on_held)(struct Joker *self, struct Card *card);
  void (*scale_independent)(struct Joker *self);
  void (*scale_on_other_jokers)(struct Joker *self, struct Joker *other);
  void (*scale_on_discard)(struct Joker *self, struct Card *card);
  void (*scale_on_blind_select)(struct Joker *self);
  void (*scale_passive)(struct Joker *self);

  union {
    double mult;
    uint16_t chips;
    uint8_t counter;
  };

  CardStatus status;
} Joker;

extern const Joker JOKERS[];
extern const uint8_t JOKER_COUNT;

#endif
