#ifndef GFX_H
#define GFX_H

#include "game.h"
#include <SDL.h>

#define CARD_SPRITE_WIDTH 48
#define CARD_SPRITE_HEIGHT 64

#ifdef __PSP__

#define CARD_WIDTH 48
#define CARD_HEIGHT 64

#else

#define CARD_WIDTH 96
#define CARD_HEIGHT 128

#endif

void render_card(Suit suit, Rank rank, SDL_Rect *dst);
void render_hand(uint8_t hovered);
void render_selected_poker_hand();

#endif
