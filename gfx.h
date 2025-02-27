#ifndef GFX_H
#define GFX_H

#include "game.h"
#include "system.h"

#define SCREEN_WIDTH (480)
#define SCREEN_HEIGHT (272)

#define CARD_WIDTH (48)
#define CARD_HEIGHT (64)

#define CHAR_WIDTH (6)
#define CHAR_HEIGHT (10)

void render_card(Suit suit, Rank rank, Rect *dst);
void render_text(const char *text, float x, float y);

void render_hand(uint8_t hovered);
void render_sidebar();

#endif
