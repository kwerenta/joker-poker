#include <math.h>
#include <pspgu.h>
#include <stdarg.h>
#include <stdio.h>

#include "debug.h"
#include "game.h"
#include "gfx.h"
#include "lib/cvector.h"
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

void render_joker(Joker *joker, Rect *dst) {
  Rect src = {.x = 9 * CARD_WIDTH, .y = CARD_HEIGHT, .w = CARD_WIDTH, .h = CARD_HEIGHT};
  if (joker->id == 6)
    src.y += 2 * CARD_HEIGHT;

  draw_texture(state.cards_atlas, &src, dst);
}

void render_consumable(Consumable *consumable, Rect *dst) {
  Rect src = {.x = 4 * CARD_WIDTH, .y = 5 * CARD_HEIGHT, .w = CARD_WIDTH, .h = CARD_HEIGHT};
  draw_texture(state.cards_atlas, &src, dst);
}

void render_spread_items(NavigationSection section, Clay_ElementId parent_id) {
  size_t item_count = 0;
  switch (section) {
  case NAVIGATION_HAND:
    item_count = cvector_size(state.game.hand.cards);
    break;

  case NAVIGATION_CONSUMABLES:
    item_count = cvector_size(state.game.consumables.items);
    break;

  case NAVIGATION_JOKERS:
    item_count = cvector_size(state.game.jokers.cards);
    break;

  default:
    log_message(LOG_WARNING, "Tried to render incompatible section in render_aligned_items function");
    return;
  }
  size_t items_width = item_count * CARD_WIDTH;

  for (uint8_t i = 0; i < item_count; i++) {
    float parent_width = Clay_GetElementData(parent_id).boundingBox.width;

    CustomElementData *element = frame_arena_allocate(sizeof(CustomElementData));
    switch (section) {
    case NAVIGATION_HAND:
      *element = (CustomElementData){.type = CUSTOM_ELEMENT_CARD, .card = state.game.hand.cards[i]};
      break;

    case NAVIGATION_CONSUMABLES:
      *element = (CustomElementData){.type = CUSTOM_ELEMENT_CONSUMABLE, .consumable = state.game.consumables.items[i]};
      break;

    case NAVIGATION_JOKERS:
      *element = (CustomElementData){.type = CUSTOM_ELEMENT_JOKER, .joker = state.game.jokers.cards[i]};
      break;

    default:
      return;
    }

    float x_offset = 0;
    if (item_count == 1)
      x_offset = (float)(parent_width - CARD_WIDTH) / 2;
    else if (items_width <= parent_width)
      x_offset = (float)(parent_width - items_width) / (item_count + 1) * (i + 1) + CARD_WIDTH * i;
    else
      x_offset = i * (float)(parent_width - CARD_WIDTH) / (item_count - 1);

    float y_offset = 0;
    if (section == NAVIGATION_HAND && state.game.hand.cards[i].selected == 1)
      y_offset = -40;

    uint8_t is_hovered = state.navigation.hovered == i && state.navigation.section == section;

    CLAY({.floating = {
              .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
              .parentId = parent_id.id,
              .offset = {.x = x_offset, .y = y_offset},
              .zIndex = is_hovered ? 2 : 1,
              .attachPoints = {.parent = CLAY_ATTACH_POINT_LEFT_CENTER, .element = CLAY_ATTACH_POINT_LEFT_CENTER},
          }}) {
      float scale = is_hovered ? 1.2 : 1;

      CLAY({.custom = {.customData = element},
            .layout = {
                .sizing = {.width = CLAY_SIZING_FIXED(scale * CARD_WIDTH),
                           .height = CLAY_SIZING_FIXED(scale * CARD_HEIGHT)},
            }}) {}
    }
  }
}

void render_hand() {
  const Hand *hand = &state.game.hand;

  CLAY({.id = CLAY_ID("Game"),
        .layout = {
            .sizing = {.width = CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .padding = {.bottom = 8},
            .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_BOTTOM},
        }}) {
    CLAY({.id = CLAY_ID("Bottom"),
          .layout = {
              .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(CARD_HEIGHT)},
              .childGap = 8,
              .layoutDirection = CLAY_LEFT_TO_RIGHT,
          }}) {
      CLAY({.id = CLAY_ID("Hand"),
            .layout = {
                .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
            }}) {}

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

  render_spread_items(NAVIGATION_HAND, CLAY_ID("Hand"));
}

void render_topbar() {
  CLAY({.id = CLAY_ID("Topbar"),
        .layout = {
            .sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(CARD_HEIGHT)},
            .childGap = 4,
        }}) {
    CLAY({.id = CLAY_ID("Jokers"),
          .backgroundColor = {30, 39, 46, 255},
          .layout = {
              .sizing = {.width = CLAY_SIZING_PERCENT(0.7), .height = CLAY_SIZING_GROW(0)},
              .childGap = 8,
              .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
          }}) {
      CLAY({.id = CLAY_ID_LOCAL("Size"),
            .floating = {
                .attachTo = CLAY_ATTACH_TO_PARENT,
                .attachPoints = {.parent = CLAY_ATTACH_POINT_LEFT_BOTTOM, .element = CLAY_ATTACH_POINT_LEFT_TOP},
            }}) {
        Clay_String size;
        append_clay_string(&size, "%d/%d", cvector_size(state.game.jokers.cards), state.game.jokers.size);

        CLAY_TEXT(size, CLAY_TEXT_CONFIG({.textColor = {255, 255, 255, 255}}));
      }
    }

    CLAY({.id = CLAY_ID("Consumables"),
          .backgroundColor = {30, 39, 46, 255},
          .layout = {
              .sizing = {.width = CLAY_SIZING_PERCENT(0.3), .height = CLAY_SIZING_FIXED(CARD_HEIGHT)},
              .childGap = 8,
              .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
          }}) {
      CLAY({.id = CLAY_ID_LOCAL("Size"),
            .floating = {
                .attachTo = CLAY_ATTACH_TO_PARENT,
                .attachPoints = {.parent = CLAY_ATTACH_POINT_RIGHT_BOTTOM, .element = CLAY_ATTACH_POINT_RIGHT_TOP},
            }}) {
        Clay_String size;
        append_clay_string(&size, "%d/%d", cvector_size(state.game.consumables.items), state.game.consumables.size);

        CLAY_TEXT(size, CLAY_TEXT_CONFIG({.textColor = {255, 255, 255, 255}}));
      }
    }
  }

  render_spread_items(NAVIGATION_JOKERS, CLAY_ID("Jokers"));
  render_spread_items(NAVIGATION_CONSUMABLES, CLAY_ID("Consumables"));
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

    CLAY({.id = CLAY_ID("Stage"),
          .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                     .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                     .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},
          .backgroundColor = {255, 168, 1, 255}}) {
      Clay_String stage;
      if (state.stage == STAGE_GAME)
        append_clay_string(&stage, "Blind %d", state.game.blind + 1);
      else
        append_clay_string(&stage, "SHOP");

      CLAY_TEXT(stage, CLAY_TEXT_CONFIG({.textColor = color_white}));
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
  CLAY({.id = CLAY_ID("Shop"),
        .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                   .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_BOTTOM},
                   .padding = {.top = 16, .left = 16, .right = 16, .bottom = 0}}}) {
    CLAY({.id = CLAY_ID("ShopContent"),
          .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                     .padding = CLAY_PADDING_ALL(16),
                     .childGap = 8,
                     .layoutDirection = CLAY_TOP_TO_BOTTOM},
          .backgroundColor = {30, 39, 46, 255}}) {

      for (uint8_t i = 0; i < cvector_size(state.game.shop.items); i++) {
        ShopItem *item = &state.game.shop.items[i];
        Clay_String name;

        switch (item->type) {
        case SHOP_ITEM_JOKER: {
          name = (Clay_String){.chars = item->joker.name, .length = strlen(item->joker.name)};
          break;
        }
        case SHOP_ITEM_CARD: {
          name = get_full_card_name(item->card.suit, item->card.rank);
          break;
        }
        case SHOP_ITEM_PLANET: {
          name = (Clay_String){.chars = get_planet_card_name(item->planet),
                               .length = strlen(get_planet_card_name(item->planet))};
          break;
        }
        case SHOP_ITEM_BOOSTER_PACK: {
          name = get_full_booster_pack_name(item->booster_pack.size, item->booster_pack.type);
          break;
        }
        }

        CLAY({.id = CLAY_IDI_LOCAL("Item", i), .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM}}) {
          Clay_Color text_color = state.navigation.section == NAVIGATION_SHOP && state.navigation.hovered == i
                                      ? (Clay_Color){0, 255, 0, 255}
                                      : (Clay_Color){255, 255, 255, 255};
          CLAY_TEXT(name, CLAY_TEXT_CONFIG({.textColor = text_color}));

          if (item->type == SHOP_ITEM_JOKER) {
            Clay_String description = {.chars = item->joker.description, .length = strlen(item->joker.description)};
            CLAY_TEXT(description, CLAY_TEXT_CONFIG({.textColor = text_color}));
          }

          Clay_String price;
          append_clay_string(&price, "$%d", get_shop_item_price(item));
          CLAY_TEXT(price, CLAY_TEXT_CONFIG({.textColor = text_color}));
        }
      }
    }
  }
}

void render_booster_pack() {
  CLAY({.id = CLAY_ID("BoosterPack"),
        .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                   .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_BOTTOM},
                   .padding = {.top = 16, .left = 16, .right = 16, .bottom = 0}}}) {
    CLAY({.id = CLAY_ID("ShopContent"),
          .layout = {.sizing = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)},
                     .padding = CLAY_PADDING_ALL(16),
                     .childGap = 8,
                     .layoutDirection = CLAY_TOP_TO_BOTTOM},
          .backgroundColor = {30, 39, 46, 255}}) {

      for (uint8_t i = 0; i < cvector_size(state.game.booster_pack.content); i++) {
        BoosterPackContent item = state.game.booster_pack.content[i];
        Clay_String name;

        switch (state.game.booster_pack.item.type) {
        case BOOSTER_PACK_STANDARD: {
          name = get_full_card_name(item.card.suit, item.card.rank);
          break;
        }
        case BOOSTER_PACK_CELESTIAL: {
          name = (Clay_String){.chars = get_planet_card_name(item.planet),
                               .length = strlen(get_planet_card_name(item.planet))};
          break;
        }
        case BOOSTER_PACK_BUFFON: {
          name = (Clay_String){.chars = item.joker.name, .length = strlen(item.joker.name)};
          break;
        }
        }

        CLAY({.id = CLAY_IDI_LOCAL("Item", i), .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM}}) {
          Clay_Color text_color = state.navigation.section == NAVIGATION_BOOSTER_PACK && state.navigation.hovered == i
                                      ? (Clay_Color){0, 255, 0, 255}
                                  : item.selected == 1 ? (Clay_Color){0, 0, 255, 255}
                                                       : (Clay_Color){255, 255, 255, 255};
          CLAY_TEXT(name, CLAY_TEXT_CONFIG({.textColor = text_color}));

          if (state.game.booster_pack.item.type == BOOSTER_PACK_BUFFON) {
            Clay_String description = {.chars = item.joker.description, .length = strlen(item.joker.description)};
            CLAY_TEXT(description, CLAY_TEXT_CONFIG({.textColor = text_color}));
          }
        }
      }
    }
  }
}

void render_game_over() {
  Vector2 pos = {10, 10};
  draw_text("You've lost:(", &pos, 0xFFFFFFFF);
}
