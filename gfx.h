#ifndef GFX_H
#define GFX_H

#include "game.h"

#define SCREEN_WIDTH (480)
#define SCREEN_HEIGHT (272)

#define CARD_WIDTH 37
#define CARD_HEIGHT 52

typedef struct {
  int x, y, w, h;
} SDL_Rect;

void render_card(Suit suit, Rank rank, SDL_Rect *dst);
// void render_text(SDL_Rect *dst, char *text, SDL_Color bg, SDL_Color fg);

void render_hand(uint8_t hovered);
void render_sidebar();

#endif
