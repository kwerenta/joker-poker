#include <math.h>
#include <pspgu.h>
#include <stdarg.h>
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

  CLAY({
      .id = CLAY_ID("Game"),
      .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                 .padding = CLAY_PADDING_ALL(8),
                 .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_BOTTOM}},
  }) {
    CLAY({
        .id = CLAY_ID("Bottom"),
        .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(CARD_HEIGHT)},
                   .childGap = 8,
                   .layoutDirection = CLAY_LEFT_TO_RIGHT},
    }) {
      CLAY({
          .id = CLAY_ID("Hand"),
          .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)}},
      }) {}

      CLAY({
          .id = CLAY_ID_LOCAL("Deck"),
          .layout = {.sizing = {.width = CLAY_SIZING_FIXED(CARD_WIDTH), .height = CLAY_SIZING_GROW(0)},
                     .childAlignment = {CLAY_ALIGN_X_RIGHT, CLAY_ALIGN_Y_BOTTOM}},
      }) {
        Clay_String deck;
        append_clay_string(&deck, "Deck %zu/%zu", cvector_size(state.game.deck), cvector_size(state.game.full_deck));

        CLAY_TEXT(deck, CLAY_TEXT_CONFIG({.textColor = {255, 255, 255, 255}, .textAlignment = CLAY_TEXT_ALIGN_CENTER}));
      }
    }
  }

  size_t card_count = cvector_size(hand->cards);
  size_t cards_width = card_count * CARD_WIDTH;

  for (uint8_t i = 0; i < card_count; i++) {
    float hand_width = Clay_GetElementData(CLAY_ID("Hand")).boundingBox.width;

    CustomElementData *card_element = frame_arena_allocate(sizeof(CustomElementData));
    *card_element = (CustomElementData){.type = CUSTOM_ELEMENT_CARD, .card = hand->cards[i]};

    float offset = 0;
    if (card_count == 1)
      offset = (float)(hand_width - CARD_WIDTH) / 2;
    else if (cards_width <= hand_width)
      offset = (float)(hand_width - cards_width) / (card_count + 1) * (i + 1) + CARD_WIDTH * i;
    else
      offset = i * (float)(hand_width - CARD_WIDTH) / (card_count - 1);

    CLAY({
        .floating =
            {
                .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                .parentId = CLAY_ID("Hand").id,
                .offset = {.x = offset, .y = hand->cards[i].selected == 1 ? -40 : 0},
                .zIndex = hovered == i ? 2 : 1,
                .attachPoints = {.parent = CLAY_ATTACH_POINT_LEFT_CENTER, .element = CLAY_ATTACH_POINT_LEFT_CENTER},
            },
    }) {
      CLAY({
          .custom = {.customData = card_element},
          .layout =
              {
                  .sizing = {.width = CLAY_SIZING_FIXED((hovered == i ? 1.2 : 1) * CARD_WIDTH),
                             .height = CLAY_SIZING_FIXED((hovered == i ? 1.2 : 1) * CARD_HEIGHT)},
              },
      }) {}
    }
  }
}

const Clay_ElementDeclaration sidebar_block = {
    .backgroundColor = {72, 84, 96, 255},
    .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
               .padding = {.top = SIDEBAR_GAP, .left = SIDEBAR_GAP, .right = SIDEBAR_GAP, .bottom = SIDEBAR_GAP},
               .layoutDirection = CLAY_TOP_TO_BOTTOM,
               .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}}};

void render_sidebar() {
  Clay_Color color_white = {255, 255, 255, 255};

  CLAY({.id = CLAY_ID("Sidebar"),
        .layout = {.sizing = {.width = CLAY_SIZING_FIXED(SIDEBAR_WIDTH), .height = CLAY_SIZING_GROW(0)},
                   .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .childGap = SIDEBAR_GAP},
        .backgroundColor = {30, 39, 46, 255}}) {

    CLAY({.id = CLAY_ID("Blind"),
          .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                     .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                     .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},
          .backgroundColor = {255, 168, 1, 255}}) {
      Clay_String blind;
      append_clay_string(&blind, "Blind %d", state.game.blind + 1);

      CLAY_TEXT(blind, CLAY_TEXT_CONFIG({.textColor = color_white}));
    }

    CLAY(sidebar_block) {
      CLAY_TEXT(CLAY_STRING("Score at least:"),
                CLAY_TEXT_CONFIG({.textColor = color_white, .wrapMode = CLAY_TEXT_WRAP_NONE}));
      Clay_String required_score;
      append_clay_string(&required_score, "%.0lf", get_required_score(state.game.ante, state.game.blind));

      CLAY_TEXT(required_score, CLAY_TEXT_CONFIG({.textColor = {255, 63, 52, 255}}));
    }

    CLAY({.id = CLAY_ID("Score"),
          .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)},
                     .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                     .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                     .childGap = SIDEBAR_GAP},
          .backgroundColor = sidebar_block.backgroundColor}) {
      CLAY({.id = CLAY_ID_LOCAL("RoundScore"),
            .layout = {.sizing = {.width = CLAY_SIZING_FIXED(5 * CHAR_WIDTH), .height = CLAY_SIZING_GROW(0)},
                       .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER}}}) {
        CLAY_TEXT(CLAY_STRING("Round score"), CLAY_TEXT_CONFIG({.textColor = color_white}));
      }

      CLAY({.id = CLAY_ID_LOCAL("ScoreValue"),
            .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                       .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}}}) {
        Clay_String score;
        append_clay_string(&score, "%.0lf", state.game.score);

        CLAY_TEXT(score, CLAY_TEXT_CONFIG({.textColor = {255, 63, 52, 255}}));
      }
    }

    Clay_ElementDeclaration sidebar_block_gap = sidebar_block;
    sidebar_block_gap.layout.childGap = SIDEBAR_GAP;

    CLAY(sidebar_block_gap) {
      Clay_String hand;
      if (state.game.selected_hand.count != 0)
        append_clay_string(&hand, "%s (%d)", get_poker_hand_name(state.game.selected_hand.hand_union),
                           state.game.poker_hands[ffs(state.game.selected_hand.hand_union) - 1] + 1);
      CLAY_TEXT(state.game.selected_hand.count == 0 ? CLAY_STRING(" ") : hand,
                CLAY_TEXT_CONFIG({.textColor = color_white, .wrapMode = CLAY_TEXT_WRAP_NONE}));

      CLAY({.id = CLAY_ID_LOCAL("Score"),
            .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                       .childGap = SIDEBAR_GAP,
                       .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}}}) {

        CLAY({.id = CLAY_ID_LOCAL("Chips"),
              .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                         .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                         .childAlignment = {CLAY_ALIGN_X_RIGHT, CLAY_ALIGN_Y_CENTER}},
              .backgroundColor = {15, 188, 249, 255}}) {
          Clay_String chips;
          append_clay_string(&chips, "%d", state.game.selected_hand.scoring.chips);

          CLAY_TEXT(state.game.selected_hand.count == 0 ? CLAY_STRING(" ") : chips,
                    CLAY_TEXT_CONFIG({.textColor = color_white}));
        }

        CLAY_TEXT(CLAY_STRING("x"), CLAY_TEXT_CONFIG({.textColor = color_white}));

        CLAY({.id = CLAY_ID_LOCAL("Mult"),
              .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                         .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                         .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER}},
              .backgroundColor = {255, 63, 52, 255}}) {
          Clay_String mult;
          append_clay_string(&mult, "%0.lf", state.game.selected_hand.scoring.mult);

          CLAY_TEXT(state.game.selected_hand.count == 0 ? CLAY_STRING(" ") : mult,
                    CLAY_TEXT_CONFIG({.textColor = color_white}));
        }
      }
    }

    CLAY(sidebar_block) {
      CLAY_TEXT(CLAY_STRING("Hands"), CLAY_TEXT_CONFIG({.textColor = color_white}));

      Clay_String hands;
      append_clay_string(&hands, "%d", state.game.hands);
      CLAY_TEXT(hands, CLAY_TEXT_CONFIG({.textColor = color_white}));
    }

    CLAY(sidebar_block) {
      CLAY_TEXT(CLAY_STRING("Discards"), CLAY_TEXT_CONFIG({.textColor = color_white}));

      Clay_String discards;
      append_clay_string(&discards, "%d", state.game.discards);
      CLAY_TEXT(discards, CLAY_TEXT_CONFIG({.textColor = color_white}));
    }

    CLAY(sidebar_block) {
      CLAY_TEXT(CLAY_STRING("Money"), CLAY_TEXT_CONFIG({.textColor = color_white}));

      Clay_String money;
      append_clay_string(&money, "$%d", state.game.money);
      CLAY_TEXT(money, CLAY_TEXT_CONFIG({.textColor = color_white}));
    }

    CLAY(
        {.id = CLAY_ID("RoundInfo"),
         .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0)}, .childGap = SIDEBAR_GAP}}) {
      CLAY(sidebar_block) {
        CLAY_TEXT(CLAY_STRING("Ante"), CLAY_TEXT_CONFIG({.textColor = color_white}));

        Clay_String ante;
        append_clay_string(&ante, "%d/8", state.game.ante);
        CLAY_TEXT(ante, CLAY_TEXT_CONFIG({.textColor = color_white}));
      }

      CLAY(sidebar_block) {
        CLAY_TEXT(CLAY_STRING("Round"), CLAY_TEXT_CONFIG({.textColor = color_white}));

        Clay_String round;
        append_clay_string(&round, "%d", state.game.round);
        CLAY_TEXT(round, CLAY_TEXT_CONFIG({.textColor = color_white}));
      }
    }
  };
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
