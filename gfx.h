#ifndef GFX_H
#define GFX_H

#include "game.h"
#include "state.h"
#include "system.h"

#define SCREEN_WIDTH (480)
#define SCREEN_HEIGHT (272)

#define CARD_WIDTH (48)
#define CARD_HEIGHT (64)

#define CHAR_WIDTH (6)
#define CHAR_HEIGHT (10)

#define SIDEBAR_WIDTH (100)
#define SIDEBAR_GAP (4)

void render_card(Card *card, Rect *dst);
void render_joker(Joker *joker, Rect *dst);
void render_consumable(Consumable *consumable, Rect *dst);

void render_spread_items(NavigationSection section, Clay_String parent_id);

void render_hand();
void render_sidebar();
void render_topbar();

void render_shop();

void render_booster_pack();

void render_cash_out();

void render_game_over();

#endif
