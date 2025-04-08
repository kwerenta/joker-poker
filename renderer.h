#ifndef RENDERER_H
#define RENDERER_H

#include "game.h"
#include "lib/clay.h"

typedef enum { CUSTOM_ELEMENT_CARD, CUSTOM_ELEMENT_JOKER, CUSTOM_ELEMENT_CONSUMABLE } CustomElementType;

typedef struct {
  CustomElementType type;
  union {
    Card card;
    Joker joker;
    Consumable consumable;
  };
} CustomElementData;

void renderer_init();
void execute_render_commands(Clay_RenderCommandArray render_commands);

#endif
