#ifndef GFX_H
#define GFX_H

#include "game.h"
#include "system.h"

#define SCREEN_WIDTH (480)
#define SCREEN_HEIGHT (272)

#define CARD_WIDTH 48
#define CARD_HEIGHT 64

void render_card(Suit suit, Rank rank, Rect *dst);
// void render_text(SDL_Rect *dst, char *text, SDL_Color bg, SDL_Color fg);

void render_hand(uint8_t hovered);
void render_sidebar();

#endif
