#ifndef JOKER_H
#define JOKER_H

#include <stdint.h>

typedef enum {
  JOKER_JOKER = 1,
  JOKER_JOLLY = 6,
} JokerId;

typedef enum { EDITION_BASE, EDITION_FOIL, EDITION_HOLOGRAPHIC, EDITION_POLYCHROME, EDITION_NEGATIVE } Edition;

typedef enum { RARITY_COMMON, RARITY_UNCOMMON, RARITY_RARE, RARITY_LEGENDARY } Rarity;

typedef enum {
  ACTIVATION_ON_PLAYED,
  ACTIVATION_ON_SCORED,
  ACTIVATION_ON_HELD,
  ACTIVATION_INDEPENDENT,
  ACTIVATION_ON_OTHER_JOKERS,
  ACTIVATION_ON_DISCARD,
  ACTIVATION_ON_BLIND_SELECT,
  ACTIVATION_PASSIVE,
} ActivationType;

typedef enum { CARD_STATUS_NORMAL = 0, CARD_STATUS_FACE_DOWN = 1 << 0, CARD_STATUS_DEBUFFED = 1 << 1 } CardStatus;

typedef struct {
  JokerId id;
  const char *name;
  const char *description;
  uint8_t base_price;

  Rarity rarity;
  Edition edition;

  ActivationType activation_type;
  void (*activate)();
  CardStatus status;
} Joker;

extern const Joker JOKERS[];
extern const uint8_t JOKER_COUNT;

#endif
