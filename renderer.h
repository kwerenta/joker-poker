#ifndef RENDERER_H
#define RENDERER_H

#include <clay.h>

#include "game.h"

typedef enum {
  CUSTOM_ELEMENT_CARD,
  CUSTOM_ELEMENT_JOKER,
  CUSTOM_ELEMENT_CONSUMABLE,
  CUSTOM_ELEMENT_VOUCHER,
  CUSTOM_ELEMENT_BOOSTER_PACK,
  CUSTOM_ELEMENT_DECK,
} CustomElementType;

typedef struct __attribute__((aligned(16))) {
  CustomElementType type;
  union {
    Card card;
    Joker joker;
    Consumable consumable;
    Voucher voucher;
    BoosterPackItem booster_pack;
    Deck deck;
  };
} CustomElementData;

void renderer_init();
void execute_render_commands(Clay_RenderCommandArray render_commands);

#endif
