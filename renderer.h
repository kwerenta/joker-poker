#ifndef RENDERER_H
#define RENDERER_H

#include "game.h"
#include "lib/clay.h"

typedef enum { CUSTOM_ELEMENT_CARD } CustomElementType;

typedef struct {
  CustomElementType type;
  union {
    Card card;
  };
} CustomElementData;

void renderer_init();
void execute_render_commands(Clay_RenderCommandArray render_commands);

#endif
