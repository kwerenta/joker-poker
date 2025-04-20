#include "gfx.h"

#include <cvector.h>
#include <math.h>
#include <pspgu.h>
#include <stdarg.h>
#include <stdio.h>

#include "game.h"
#include "renderer.h"
#include "state.h"
#include "system.h"
#include "text.h"
#include "utils.h"

void render_card_atlas_sprite(Vector2 *sprite_index, Rect *dst) {
  float angle = 3.0f * sinf(state.delta * 1.0f - dst->x / SCREEN_WIDTH * M_PI * 3);
  Rect src = {.x = sprite_index->x * CARD_WIDTH, .y = sprite_index->y * CARD_HEIGHT, .w = CARD_WIDTH, .h = CARD_HEIGHT};

  draw_texture(state.cards_atlas, &src, dst, 0xFFFFFFFF, angle);
}

void render_card(Card *card, Rect *dst) {
  Vector2 background = {.x = 9, .y = 7};
  if (card->enhancement != ENHANCEMENT_NONE) {
    uint8_t enhancement_offset = card->enhancement - 1;
    background.x = 5 + enhancement_offset % 4;
    background.y = 5 + 2 * floor(enhancement_offset / 4.0);
  }
  render_card_atlas_sprite(&background, dst);

  Vector2 face = {.x = card->rank % 10, .y = 2 * card->suit + floor(card->rank / 10.0)};
  if (card->enhancement != ENHANCEMENT_STONE) render_card_atlas_sprite(&face, dst);

  Vector2 edition = {.x = 5 + card->edition - 1, .y = 3};
  if (card->edition != EDITION_BASE) render_card_atlas_sprite(&edition, dst);
}

void render_joker(Joker *joker, Rect *dst) {
  Vector2 src = {.x = 9, .y = 1};
  if (joker->id == 6) src.y += 2;

  render_card_atlas_sprite(&src, dst);
}

void render_consumable(Consumable *consumable, Rect *dst) { render_card_atlas_sprite(&(Vector2){.x = 4, .y = 5}, dst); }

void render_booster_pack(BoosterPackItem *booster_pack, Rect *dst) {
  render_card_atlas_sprite(&(Vector2){.x = 4, .y = 7}, dst);
}

void render_spread_items(NavigationSection section, Clay_String parent_id) {
  size_t item_count = get_nav_section_size(section);
  if (item_count == 0) return;
  size_t items_width = item_count * CARD_WIDTH;

  for (uint8_t i = 0; i < item_count; i++) {
    float parent_width = Clay_GetElementData(CLAY_SID(parent_id)).boundingBox.width;
    CustomElementData *element = frame_arena_allocate(sizeof(CustomElementData));
    *element = create_spread_item_element(section, i);

    float x_offset = 0;
    if (item_count == 1)
      x_offset = (float)(parent_width - CARD_WIDTH) / 2;
    else if (items_width <= parent_width)
      x_offset = (float)(parent_width - items_width) / (item_count + 1) * (i + 1) + CARD_WIDTH * i;
    else
      x_offset = i * (float)(parent_width - CARD_WIDTH) / (item_count - 1);

    float y_offset = 0;
    if (section == NAVIGATION_HAND && state.game.hand.cards[i].selected == 1) y_offset = -40;

    uint8_t is_hovered = state.navigation.hovered == i && state.navigation.section == section;

    CLAY({.id = CLAY_SIDI(parent_id, i + 1),
          .floating = {
              .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
              .parentId = CLAY_SID(parent_id).id,
              .offset = {.x = x_offset, .y = y_offset},
              .zIndex = is_hovered ? 2 : 1,
              .attachPoints = {.parent = CLAY_ATTACH_POINT_LEFT_CENTER, .element = CLAY_ATTACH_POINT_LEFT_CENTER},
          }}) {
      float scale = is_hovered ? 1.2 : 1;

      CLAY({.custom = {.customData = element},
            .layout = {
                .sizing = {CLAY_SIZING_FIXED(scale * CARD_WIDTH), CLAY_SIZING_FIXED(scale * CARD_HEIGHT)},
            }}) {
        if (section != NAVIGATION_SHOP_ITEMS && section != NAVIGATION_SHOP_BOOSTER_PACKS) continue;

        CLAY({.id = CLAY_ID_LOCAL("Price"),
              .floating = {.attachTo = CLAY_ATTACH_TO_PARENT,
                           .offset = {.y = CHAR_HEIGHT},
                           .zIndex = 4,
                           .attachPoints = {.parent = CLAY_ATTACH_POINT_CENTER_TOP,
                                            .element = CLAY_ATTACH_POINT_CENTER_BOTTOM}}}) {
          CLAY({.backgroundColor = COLOR_CARD_BG,
                .border = {.color = COLOR_MONEY, .width = CLAY_BORDER_ALL(1)},
                .layout = {
                    .padding = CLAY_PADDING_ALL(4),
                }}) {
            Clay_String price;
            append_clay_string(&price, "$%d",
                               section == NAVIGATION_SHOP_BOOSTER_PACKS
                                   ? get_booster_pack_price(&state.game.shop.booster_packs[i])
                                   : get_shop_item_price(&state.game.shop.items[i]));
            CLAY_TEXT(price, WHITE_TEXT_CONFIG);
          }
        }
      }
    }
  }

  if (state.navigation.section != section) return;

  Clay_String name;
  Clay_String description;
  get_nav_item_tooltip_content(&name, &description, section);

  float y_offset = 4;
  Clay_FloatingAttachPoints attach_points = {.parent = CLAY_ATTACH_POINT_CENTER_BOTTOM,
                                             .element = CLAY_ATTACH_POINT_CENTER_TOP};

  if (section == NAVIGATION_HAND || section == NAVIGATION_SHOP_ITEMS || section == NAVIGATION_SHOP_BOOSTER_PACKS ||
      section == NAVIGATION_BOOSTER_PACK) {
    y_offset *= -1;
    attach_points.parent = CLAY_ATTACH_POINT_CENTER_TOP;
    attach_points.element = CLAY_ATTACH_POINT_CENTER_BOTTOM;
  }

  CLAY({.id = CLAY_ID("Tooltip"),
        .floating = {
            .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
            .parentId = CLAY_SIDI(parent_id, state.navigation.hovered + 1).id,
            .zIndex = 3,
            .offset = {.y = y_offset},
            .attachPoints = attach_points,
        }}) {
    CLAY({.backgroundColor = COLOR_CARD_BG,
          .layout = {
              .padding = CLAY_PADDING_ALL(4),
              .childGap = 2,
              .sizing = {CLAY_SIZING_GROW(0, 100), CLAY_SIZING_GROW(0)},
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
          }}) {
      CLAY_TEXT(name, WHITE_TEXT_CONFIG);
      CLAY_TEXT(description, WHITE_TEXT_CONFIG);
    }
  }
}

void render_hand() {
  const Hand *hand = &state.game.hand;

  CLAY({.id = CLAY_ID("Game"),
        .layout = {
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .padding = {.bottom = 8},
            .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_BOTTOM},
        }}) {
    CLAY({.id = CLAY_ID_LOCAL("Bottom"),
          .layout = {
              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CARD_HEIGHT)},
              .childGap = 8,
              .layoutDirection = CLAY_LEFT_TO_RIGHT,
          }}) {
      CLAY({.id = CLAY_ID("Hand"),
            .layout = {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            }}) {}

      CLAY({
          .id = CLAY_ID_LOCAL("Deck"),
          .layout = {.sizing = {CLAY_SIZING_FIXED(CARD_WIDTH), CLAY_SIZING_GROW(0)},
                     .childAlignment = {CLAY_ALIGN_X_RIGHT, CLAY_ALIGN_Y_BOTTOM}},
      }) {
        Clay_String deck;
        append_clay_string(&deck, "Deck %zu/%zu", cvector_size(state.game.deck), cvector_size(state.game.full_deck));

        CLAY_TEXT(deck, CLAY_TEXT_CONFIG({.textColor = COLOR_WHITE, .textAlignment = CLAY_TEXT_ALIGN_CENTER}));
      }
    }
  }

  render_spread_items(NAVIGATION_HAND, CLAY_STRING("Hand"));
}

void render_topbar() {
  CLAY({.id = CLAY_ID("Topbar"),
        .layout = {
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CARD_HEIGHT)},
            .childGap = 4,
        }}) {
    CLAY({.id = CLAY_ID("Jokers"),
          .backgroundColor = COLOR_CARD_BG,
          .layout = {
              .sizing = {CLAY_SIZING_PERCENT(0.7), CLAY_SIZING_GROW(0)},
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

        CLAY_TEXT(size, WHITE_TEXT_CONFIG);
      }
    }

    CLAY({.id = CLAY_ID("Consumables"),
          .backgroundColor = COLOR_CARD_BG,
          .layout = {
              .sizing = {CLAY_SIZING_PERCENT(0.3), CLAY_SIZING_FIXED(CARD_HEIGHT)},
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

        CLAY_TEXT(size, WHITE_TEXT_CONFIG);
      }
    }
  }

  render_spread_items(NAVIGATION_JOKERS, CLAY_STRING("Jokers"));
  render_spread_items(NAVIGATION_CONSUMABLES, CLAY_STRING("Consumables"));
}

const Clay_ElementDeclaration sidebar_block_config = {
    .backgroundColor = {72, 84, 96, 255},
    .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)},
               .padding = {.top = SIDEBAR_GAP, .left = SIDEBAR_GAP, .right = SIDEBAR_GAP, .bottom = SIDEBAR_GAP},
               .layoutDirection = CLAY_TOP_TO_BOTTOM,
               .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}}};

void render_sidebar() {
  CLAY({.id = CLAY_ID("Sidebar"),
        .layout = {.sizing = {CLAY_SIZING_FIXED(SIDEBAR_WIDTH), CLAY_SIZING_GROW(0)},
                   .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .childGap = SIDEBAR_GAP},
        .backgroundColor = COLOR_CARD_BG}) {
    CLAY({.id = CLAY_ID("Stage"),
          .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                     .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                     .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},
          .backgroundColor = COLOR_MONEY}) {
      Clay_String stage;
      if (state.stage == STAGE_GAME)
        append_clay_string(&stage, "Blind %d", state.game.blind + 1);
      else
        append_clay_string(&stage, "SHOP");

      CLAY_TEXT(stage, WHITE_TEXT_CONFIG);
    }

    CLAY(sidebar_block_config) {
      CLAY_TEXT(CLAY_STRING("Score at least:"),
                CLAY_TEXT_CONFIG({.textColor = COLOR_WHITE, .wrapMode = CLAY_TEXT_WRAP_NONE}));
      Clay_String required_score;
      append_clay_string(&required_score, "%.0lf", get_required_score(state.game.ante, state.game.blind));

      CLAY_TEXT(required_score, CLAY_TEXT_CONFIG({.textColor = {255, 63, 52, 255}}));
    }

    CLAY({.id = CLAY_ID("Score"),
          .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)},
                     .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                     .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                     .childGap = SIDEBAR_GAP},
          .backgroundColor = sidebar_block_config.backgroundColor}) {
      CLAY({.id = CLAY_ID_LOCAL("RoundScore"),
            .layout = {.sizing = {CLAY_SIZING_FIXED(5 * CHAR_WIDTH), CLAY_SIZING_GROW(0)},
                       .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER}}}) {
        CLAY_TEXT(CLAY_STRING("Round score"), WHITE_TEXT_CONFIG);
      }

      CLAY({.id = CLAY_ID_LOCAL("ScoreValue"),
            .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                       .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}}}) {
        Clay_String score;
        append_clay_string(&score, "%.0lf", state.game.score);

        CLAY_TEXT(score, CLAY_TEXT_CONFIG({.textColor = {255, 63, 52, 255}}));
      }
    }

    Clay_ElementDeclaration sidebar_block_gap = sidebar_block_config;
    sidebar_block_gap.layout.childGap = SIDEBAR_GAP;

    CLAY(sidebar_block_gap) {
      Clay_String hand;
      if (state.game.selected_hand.count != 0)
        append_clay_string(&hand, "%s (%d)", get_poker_hand_name(state.game.selected_hand.hand_union),
                           state.game.poker_hands[ffs(state.game.selected_hand.hand_union) - 1] + 1);
      CLAY_TEXT(state.game.selected_hand.count == 0 ? CLAY_STRING(" ") : hand,
                CLAY_TEXT_CONFIG({.textColor = COLOR_WHITE, .wrapMode = CLAY_TEXT_WRAP_NONE}));

      CLAY({.id = CLAY_ID_LOCAL("Score"),
            .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                       .childGap = SIDEBAR_GAP,
                       .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}}}) {
        CLAY({.id = CLAY_ID_LOCAL("Chips"),
              .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                         .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                         .childAlignment = {CLAY_ALIGN_X_RIGHT, CLAY_ALIGN_Y_CENTER}},
              .backgroundColor = COLOR_CHIPS}) {
          Clay_String chips;
          append_clay_string(&chips, "%d", state.game.selected_hand.score_pair.chips);

          CLAY_TEXT(state.game.selected_hand.count == 0 ? CLAY_STRING(" ") : chips, WHITE_TEXT_CONFIG);
        }

        CLAY_TEXT(CLAY_STRING("x"), WHITE_TEXT_CONFIG);

        CLAY({.id = CLAY_ID_LOCAL("Mult"),
              .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                         .padding = CLAY_PADDING_ALL(SIDEBAR_GAP),
                         .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER}},
              .backgroundColor = COLOR_MULT}) {
          Clay_String mult;
          append_clay_string(&mult, "%0.lf", state.game.selected_hand.score_pair.mult);

          CLAY_TEXT(state.game.selected_hand.count == 0 ? CLAY_STRING(" ") : mult, WHITE_TEXT_CONFIG);
        }
      }
    }

    CLAY(sidebar_block_config) {
      CLAY_TEXT(CLAY_STRING("Hands"), WHITE_TEXT_CONFIG);

      Clay_String hands;
      append_clay_string(&hands, "%d", state.game.hands);
      CLAY_TEXT(hands, WHITE_TEXT_CONFIG);
    }

    CLAY(sidebar_block_config) {
      CLAY_TEXT(CLAY_STRING("Discards"), WHITE_TEXT_CONFIG);

      Clay_String discards;
      append_clay_string(&discards, "%d", state.game.discards);
      CLAY_TEXT(discards, WHITE_TEXT_CONFIG);
    }

    CLAY(sidebar_block_config) {
      CLAY_TEXT(CLAY_STRING("Money"), WHITE_TEXT_CONFIG);

      Clay_String money;
      append_clay_string(&money, "$%d", state.game.money);
      CLAY_TEXT(money, WHITE_TEXT_CONFIG);
    }

    CLAY({.id = CLAY_ID("RoundInfo"),
          .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)}, .childGap = SIDEBAR_GAP}}) {
      CLAY(sidebar_block_config) {
        CLAY_TEXT(CLAY_STRING("Ante"), WHITE_TEXT_CONFIG);

        Clay_String ante;
        append_clay_string(&ante, "%d/8", state.game.ante);
        CLAY_TEXT(ante, WHITE_TEXT_CONFIG);
      }

      CLAY(sidebar_block_config) {
        CLAY_TEXT(CLAY_STRING("Round"), WHITE_TEXT_CONFIG);

        Clay_String round;
        append_clay_string(&round, "%d", state.game.round);
        CLAY_TEXT(round, WHITE_TEXT_CONFIG);
      }
    }
  };
}

Clay_ElementDeclaration card_element_config(Clay_ElementId id) {
  return (Clay_ElementDeclaration){.id = id,
                                   .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                                              .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_BOTTOM},
                                              .padding = {.top = 16, .left = 16, .right = 16, .bottom = 0}}};
};

Clay_ElementDeclaration card_content_config() {
  return (Clay_ElementDeclaration){.id = CLAY_ID_LOCAL("Content"),
                                   .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                                              .padding = CLAY_PADDING_ALL(16),
                                              .childGap = 8,
                                              .layoutDirection = CLAY_TOP_TO_BOTTOM},
                                   .backgroundColor = COLOR_CARD_BG};
}

void render_shop() {
  CLAY(card_element_config(CLAY_ID("Shop"))) {
    CLAY(card_content_config()) {
      CLAY({.id = CLAY_ID("ShopItems"), .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CARD_HEIGHT)}}}) {}
      render_spread_items(NAVIGATION_SHOP_ITEMS, CLAY_STRING("ShopItems"));

      CLAY({.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}}}) {}

      CLAY({.id = CLAY_ID("ShopBoosterPacks"),
            .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CARD_HEIGHT)}}}) {}
      render_spread_items(NAVIGATION_SHOP_BOOSTER_PACKS, CLAY_STRING("ShopBoosterPacks"));
    }
  }
}

void render_booster_pack_content() {
  CLAY({.id = CLAY_ID("BoosterPack"),
        .layout = {
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .childGap = 8,
            .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_BOTTOM},
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        }}) {
    CLAY({.id = CLAY_ID("BoosterPackItems"),
          .layout = {
              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CARD_HEIGHT)},
          }}) {}

    CLAY({.id = CLAY_ID_LOCAL("Name"),
          .layout = {.sizing = CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0), .padding = CLAY_PADDING_ALL(4)},
          .backgroundColor = COLOR_CARD_BG}) {
      Clay_String name =
          get_full_booster_pack_name(state.game.booster_pack.item.size, state.game.booster_pack.item.type);
      CLAY_TEXT(name, WHITE_TEXT_CONFIG);
    }
  }

  render_spread_items(NAVIGATION_BOOSTER_PACK, CLAY_STRING("BoosterPackItems"));
}

void render_cash_out() {
  CLAY(card_element_config(CLAY_ID("Cashout"))) {
    CLAY(card_content_config()) {
      uint8_t interest = get_interest_money();
      uint8_t hands = get_hands_money();
      uint8_t blind = get_blind_money(state.game.blind);

      CLAY({.layout = {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)},
                .childAlignment = {CLAY_ALIGN_X_CENTER},
            }}) {
        Clay_String cash_out_text;
        append_clay_string(&cash_out_text, "Cash Out: $%d", interest + hands + blind);
        CLAY_TEXT(cash_out_text, CLAY_TEXT_CONFIG({.textColor = COLOR_MONEY}));
      }

      CLAY({.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(1)}}, .backgroundColor = COLOR_WHITE}) {}

      CLAY({.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM}}) {
        if (blind != 0) {
          Clay_String blind_money;
          append_clay_string(&blind_money, "Blind: $%d", get_blind_money(state.game.blind));
          CLAY_TEXT(blind_money, WHITE_TEXT_CONFIG);
        }

        if (hands != 0) {
          Clay_String hands_money;
          append_clay_string(&hands_money, "Hands: $%d", get_hands_money());
          CLAY_TEXT(hands_money, WHITE_TEXT_CONFIG);
        }

        if (interest != 0) {
          Clay_String interest_money;
          append_clay_string(&interest_money, "Interest: $%d", get_interest_money());
          CLAY_TEXT(interest_money, WHITE_TEXT_CONFIG);
        }
      }
    }
  }
}

void render_game_over() {
  Vector2 pos = {10, 10};
  draw_text("You've lost:(", &pos, 0xFFFFFFFF);
}
