#include <math.h>
#include <pspgu.h>
#include <stdio.h>

#include "game.h"
#include "gfx.h"
#include "renderer.h"
#include "state.h"
#include "system.h"
#include "text.h"

void render_card(Card *card, Rect *dst) {
  Rect background = {.x = 9 * CARD_WIDTH, .y = 7 * CARD_HEIGHT, .w = CARD_WIDTH, .h = CARD_HEIGHT};
  if (card->enhancement != ENHANCEMENT_NONE) {
    uint8_t enhancement_offset = card->enhancement - 1;
    background.x = (5 + enhancement_offset % 4) * CARD_WIDTH;
    background.y = (5 + 2 * floor(enhancement_offset / 4.0)) * CARD_HEIGHT;
  }
  draw_texture(state.cards_atlas, &background, dst);

  Rect face = {.x = (card->rank % 10) * CARD_WIDTH,
               .y = (2 * card->suit + floor(card->rank / 10.0)) * CARD_HEIGHT,
               .w = CARD_WIDTH,
               .h = CARD_HEIGHT};
  if (card->enhancement != ENHANCEMENT_STONE)
    draw_texture(state.cards_atlas, &face, dst);

  if (card->edition != EDITION_BASE) {
    Rect edition = {.x = (5 + card->edition - 1) * CARD_WIDTH, .y = 3 * CARD_HEIGHT, .w = CARD_WIDTH, .h = CARD_HEIGHT};
    draw_texture(state.cards_atlas, &edition, dst);
  }
}

void render_hand(uint8_t hovered) {
  const Hand *hand = &state.game.hand;

  uint32_t hand_width = cvector_size(hand->cards) * (CARD_WIDTH - 16);
  for (uint8_t i = 0; i < cvector_size(hand->cards); i++) {
    if (hovered == i) {
      continue;
    }

    Rect dst = {.x = SCREEN_WIDTH / 2.0 - hand_width / 2.0 + (CARD_WIDTH - 16) * i,
                .y = SCREEN_HEIGHT - CARD_HEIGHT - 16,
                .w = CARD_WIDTH,
                .h = CARD_HEIGHT};

    if (hand->cards[i].selected == 1) {
      dst.y -= 50;
    }

    render_card(&hand->cards[i], &dst);
  }

  render_card(
      &hand->cards[hovered],
      &(Rect){.x = SCREEN_WIDTH / 2.0 - hand_width / 2.0 + (CARD_WIDTH - 16) * hovered - CARD_WIDTH * 0.1,
              .y = SCREEN_HEIGHT - CARD_HEIGHT - 16 - CARD_HEIGHT * 0.1 - (hand->cards[hovered].selected == 1 ? 50 : 0),
              .w = CARD_WIDTH * 1.2,
              .h = CARD_HEIGHT * 1.2});
}

void render_sidebar() {
  char buffer[64];
  Clay_Color color_white = {255, 255, 255, 255};
  Clay_Color color_block_bg = {72, 84, 96, 255};
  uint32_t white = 0xFFFFFFFF;

  CLAY({.id = CLAY_ID("Sidebar"),
        .layout = {.sizing = {.width = CLAY_SIZING_FIXED(SIDEBAR_WIDTH), .height = CLAY_SIZING_GROW(0)},
                   .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .childGap = SIDEBAR_GAP},
        .backgroundColor = {30, 39, 46, 255}}){

      CLAY({.id = CLAY_ID("Blind"),
            .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                       .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                       .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},
            .backgroundColor = {255, 168, 1, 255}}){

          CLAY_TEXT(CLAY_STRING("Blind %d"), CLAY_TEXT_CONFIG({.textColor = color_white}));
}

CLAY({.id = CLAY_ID("RequiredScore"),
      .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
                 .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                 .layoutDirection = CLAY_TOP_TO_BOTTOM,
                 .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},
      .backgroundColor = color_block_bg}) {
  CLAY_TEXT(CLAY_STRING("Score at least:"),
            CLAY_TEXT_CONFIG({.textColor = color_white, .wrapMode = CLAY_TEXT_WRAP_NONE}));
  CLAY_TEXT(CLAY_STRING("%d"), CLAY_TEXT_CONFIG({.textColor = {255, 63, 52, 255}}));
}

CLAY({.id = CLAY_ID("Score"),
      .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
                 .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                 .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                 .childGap = SIDEBAR_GAP},
      .backgroundColor = color_block_bg}) {
  CLAY({.id = CLAY_ID_LOCAL("RoundScore"),
        .layout = {.sizing = {.width = CLAY_SIZING_FIXED(5 * CHAR_WIDTH), .height = CLAY_SIZING_GROW(0)},
                   .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER}}}) {
    CLAY_TEXT(CLAY_STRING("Round score"), CLAY_TEXT_CONFIG({.textColor = color_white}));
  }

  CLAY({.id = CLAY_ID_LOCAL("ScoreValue"),
        .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                   .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}}}) {
    CLAY_TEXT(CLAY_STRING("%.0lf"), CLAY_TEXT_CONFIG({.textColor = {255, 63, 52, 255}}));
  }
}

CLAY({.id = CLAY_ID("CurrentHand"),
      .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
                 .layoutDirection = CLAY_TOP_TO_BOTTOM,
                 .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                 .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                 .childGap = SIDEBAR_GAP},
      .backgroundColor = color_block_bg}) {

  CLAY_TEXT(state.game.selected_hand.count == 0 ? CLAY_STRING(" ") : CLAY_STRING("%s (%d)"),
            CLAY_TEXT_CONFIG({.textColor = color_white, .wrapMode = CLAY_TEXT_WRAP_NONE}));

  CLAY({.id = CLAY_ID_LOCAL("Score"),
        .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                   .childGap = SIDEBAR_GAP,
                   .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}}}) {

    CLAY({.id = CLAY_ID_LOCAL("Chips"),
          .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                     .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                     .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},

          .backgroundColor = {15, 188, 249, 255}}) {
      CLAY_TEXT(state.game.selected_hand.count == 0 ? CLAY_STRING(" ") : CLAY_STRING("%0.l"),
                CLAY_TEXT_CONFIG({.textColor = color_white}));
    }

    CLAY_TEXT(CLAY_STRING("x"), CLAY_TEXT_CONFIG({.textColor = color_white}));

    CLAY({.id = CLAY_ID_LOCAL("Mult"),
          .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                     .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                     .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},
          .backgroundColor = {255, 63, 52, 255}}) {
      CLAY_TEXT(state.game.selected_hand.count == 0 ? CLAY_STRING(" ") : CLAY_STRING("%0.lf"),
                CLAY_TEXT_CONFIG({.textColor = color_white}));
    }
  }
}

CLAY({.id = CLAY_ID("Hands"),
      .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
                 .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                 .layoutDirection = CLAY_TOP_TO_BOTTOM,
                 .padding = CLAY_PADDING_ALL(SIDEBAR_GAP)},
      .backgroundColor = color_block_bg}) {
  CLAY_TEXT(CLAY_STRING("Hands"), CLAY_TEXT_CONFIG({.textColor = color_white}));
  CLAY_TEXT(CLAY_STRING("%d"), CLAY_TEXT_CONFIG({.textColor = color_white}));
}

CLAY({.id = CLAY_ID("Discards"),
      .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
                 .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                 .layoutDirection = CLAY_TOP_TO_BOTTOM,
                 .padding = CLAY_PADDING_ALL(SIDEBAR_GAP)},
      .backgroundColor = color_block_bg}) {
  CLAY_TEXT(CLAY_STRING("Discards"), CLAY_TEXT_CONFIG({.textColor = color_white}));
  CLAY_TEXT(CLAY_STRING("%d"), CLAY_TEXT_CONFIG({.textColor = color_white}));
}

CLAY({.id = CLAY_ID("Money"),
      .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
                 .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                 .layoutDirection = CLAY_TOP_TO_BOTTOM,
                 .padding = CLAY_PADDING_ALL(SIDEBAR_GAP)},
      .backgroundColor = color_block_bg}) {
  CLAY_TEXT(CLAY_STRING("Money"), CLAY_TEXT_CONFIG({.textColor = color_white}));
  CLAY_TEXT(CLAY_STRING("$%d"), CLAY_TEXT_CONFIG({.textColor = color_white}));
}

CLAY({.id = CLAY_ID("RoundInfo"),
      .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)}, .childGap = SIDEBAR_GAP}}) {
  CLAY({.id = CLAY_ID_LOCAL("Ante"),
        {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
         .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
         .layoutDirection = CLAY_TOP_TO_BOTTOM,
         .padding = CLAY_PADDING_ALL(SIDEBAR_GAP)},
        .backgroundColor = color_block_bg}) {
    CLAY_TEXT(CLAY_STRING("Ante"), CLAY_TEXT_CONFIG({.textColor = color_white}));
    CLAY_TEXT(CLAY_STRING("%d/8"), CLAY_TEXT_CONFIG({.textColor = color_white}));
  }

  CLAY({.id = CLAY_ID_LOCAL("Round"),
        {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
         .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
         .layoutDirection = CLAY_TOP_TO_BOTTOM,
         .padding = CLAY_PADDING_ALL(SIDEBAR_GAP)},
        .backgroundColor = color_block_bg}) {
    CLAY_TEXT(CLAY_STRING("Round"), CLAY_TEXT_CONFIG({.textColor = color_white}));
    CLAY_TEXT(CLAY_STRING("%d"), CLAY_TEXT_CONFIG({.textColor = color_white}));
  }
}
}
;

snprintf(buffer, 64, "Deck: %zu/%zu", cvector_size(state.game.deck), cvector_size(state.game.full_deck));
draw_text(buffer, &(Vector2){400, 254}, white);
}

void render_shop() {
  char buffer[64];
  Vector2 pos = {10, 10};

  snprintf(buffer, 64, "Money: $%d", state.game.money);
  draw_text(buffer, &pos, 0xFFFFFFFF);

  for (uint8_t i = 0; i < cvector_size(state.game.shop.items); i++) {
    uint32_t color = state.game.shop.selected_card == i ? 0xFF00FF00 : 0xFFFFFFFF;
    ShopItem *item = &state.game.shop.items[i];

    pos.y += 16;

    switch (item->type) {
    case SHOP_ITEM_JOKER:
      draw_text(item->joker.name, &pos, color);

      pos.y += 8;
      draw_text(item->joker.description, &pos, color);
      break;

    case SHOP_ITEM_CARD:
      get_full_card_name(buffer, item->card.suit, item->card.rank);
      draw_text(buffer, &pos, color);
      break;

    case SHOP_ITEM_PLANET:
      draw_text(get_planet_card_name(item->planet), &pos, color);
      break;

    case SHOP_ITEM_BOOSTER_PACK:
      get_full_booster_pack_name(buffer, item->booster_pack.size, item->booster_pack.type);
      draw_text(buffer, &pos, color);
      break;
    }

    pos.y += 8;
    snprintf(buffer, 64, "$%d", get_shop_item_price(item));
    draw_text(buffer, &pos, color);
  }
}

void render_booster_pack() {
  char buffer[64];
  Vector2 pos = {10, 10};

  snprintf(buffer, 64, "Money: $%d", state.game.money);
  draw_text(buffer, &pos, 0xFFFFFFFF);

  for (uint8_t i = 0; i < cvector_size(state.game.booster_pack.content); i++) {
    BoosterPackContent item = state.game.booster_pack.content[i];
    uint32_t color = state.game.booster_pack.hovered_item == i ? 0xFF00FF00
                     : item.selected == 1                      ? 0xFFFF0000
                                                               : 0xFFFFFFFF;

    pos.y += 16;

    switch (state.game.booster_pack.item.type) {
    case BOOSTER_PACK_BUFFON:
      draw_text(item.joker.name, &pos, color);

      pos.y += 8;
      draw_text(item.joker.description, &pos, color);
      break;

    case BOOSTER_PACK_STANDARD:
      get_full_card_name(buffer, item.card.suit, item.card.rank);
      draw_text(buffer, &pos, color);
      break;

    case BOOSTER_PACK_CELESTIAL:
      draw_text(get_planet_card_name(item.planet), &pos, color);
      break;
    }
  }
}

void render_game_over() {
  Vector2 pos = {10, 10};
  draw_text("You've lost:(", &pos, 0xFFFFFFFF);
}
