#ifndef GFX_H
#define GFX_H

#include "game.h"
#include "state.h"
#include "system.h"

#define SCREEN_WIDTH (480)
#define SCREEN_HEIGHT (272)

#define BG_NOISE_SIZE 256
#define BG_PERIOD 256
#define BG_TEXTURE_WIDTH (60)
#define BG_TEXTURE_HEIGHT (34)

#define CARD_WIDTH (48)
#define CARD_HEIGHT (64)

#define CHAR_WIDTH (6)
#define CHAR_HEIGHT (10)

#define SIDEBAR_WIDTH (100)
#define SIDEBAR_GAP (4)
#define SECTION_PADDING (4)

#define COLOR_WHITE (Clay_Color){255, 255, 255, 255}
#define COLOR_BLACK (Clay_Color){0, 0, 0, 255}
#define COLOR_MULT (Clay_Color){255, 63, 52, 255}
#define COLOR_CHIPS (Clay_Color){15, 188, 249, 255}
#define COLOR_MONEY (Clay_Color){255, 168, 1, 255}
#define COLOR_CARD_BG_ALPHA(alpha) \
  (Clay_Color) { 30, 39, 46, alpha }
#define COLOR_CARD_BG COLOR_CARD_BG_ALPHA(255)
#define COLOR_SECTION_BG (Clay_Color){0, 0, 0, 60}
#define COLOR_CARD_LIGHT_BG (Clay_Color){72, 84, 96, 255}

#define WHITE_TEXT_CONFIG CLAY_TEXT_CONFIG({.textColor = COLOR_WHITE})

void update_render_commands();

void render_main_menu();
void render_select_deck();
void render_credits();

void render_card_atlas_sprite(Vector2 *sprite_index, Rect *dst);
void render_card(Card *card, Rect *dst);
void render_joker(Joker *joker, Rect *dst);
void render_consumable(Consumable *consumable, Rect *dst);
void render_voucher(Voucher voucher, Rect *dst);
void render_booster_pack(BoosterPackItem *booster_pack, Rect *dst);
void render_deck(Deck deck, Rect *dst);

void render_spread_items(NavigationSection section, Clay_String parent_id);

void render_hand();
void render_sidebar();
void render_topbar();

void render_shop();

void render_booster_pack_content();

void render_cash_out();
void render_select_blind();

void render_game_over();

void render_overlay_menu();
void render_overlay_select_stake();
void render_overlay_poker_hands();

void render_background();
void init_background();

#endif
