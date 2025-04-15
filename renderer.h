#ifndef RENDERER_H
#define RENDERER_H

#include "game.h"
#include "lib/clay.h"

typedef enum {
  CUSTOM_ELEMENT_CARD,
  CUSTOM_ELEMENT_JOKER,
  CUSTOM_ELEMENT_CONSUMABLE,
  CUSTOM_ELEMENT_BOOSTER_PACK
} CustomElementType;

typedef struct __attribute__((aligned(16))) {
  CustomElementType type;
  union {
    Card card;
    Joker joker;
    Consumable consumable;
    BoosterPackItem booster_pack;
  };
} CustomElementData;

void renderer_init();
void execute_render_commands(Clay_RenderCommandArray render_commands);

#endif
