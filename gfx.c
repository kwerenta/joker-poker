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

void update_render_commands() {
  Clay_BeginLayout();

  CLAY({.id = CLAY_ID("Container"), .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}}}) {
    if (state.stage != STAGE_GAME_OVER && state.stage >= STAGE_GAME) render_sidebar();

    CLAY({.id = CLAY_ID("Content"),
          .layout = {
              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
              .padding = {.top = 8, .left = 8, .right = 8, .bottom = 0},
          }}) {
      if (state.stage != STAGE_GAME_OVER && state.stage >= STAGE_GAME) render_topbar();

      switch (state.stage) {
        case STAGE_MAIN_MENU:
          render_main_menu();
          break;
        case STAGE_SELECT_DECK:
          render_select_deck();
          break;
        case STAGE_CREDITS:
          render_credits();
          break;

        case STAGE_GAME:
          render_hand();
          break;
        case STAGE_CASH_OUT:
          render_cash_out();
          break;
        case STAGE_SHOP:
          render_shop();
          break;
        case STAGE_SELECT_BLIND:
          render_select_blind();
          break;
        case STAGE_GAME_OVER:
          render_game_over();
          break;
        case STAGE_BOOSTER_PACK:
          render_booster_pack_content();
          break;
      }
    }
  }

  switch (state.overlay) {
    case OVERLAY_NONE:
      break;
    case OVERLAY_MENU:
      render_overlay_menu();
      break;
    case OVERLAY_SELECT_STAKE:
      render_overlay_select_stake();
      break;
    case OVERLAY_POKER_HANDS:
      render_overlay_poker_hands();
      break;
  }

  state.render_commands = Clay_EndLayout();
}

const Clay_String main_menu_buttons[] = {CLAY_STRING("Play"), CLAY_STRING("Credits"), CLAY_STRING("Quit")};

void render_main_menu() {
  CLAY({.layout = {
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .padding = CLAY_PADDING_ALL(16),
        }}) {
    CLAY({.layout = {
              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
              .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_BOTTOM},
          }}) {
      CLAY({.layout = {.sizing = {CLAY_SIZING_FIXED(256), CLAY_SIZING_FIXED(64)}},
            .image = {.imageData = state.logo}}) {}
    }

    CLAY({.id = CLAY_ID("MainMenuButtons"),
          .layout = {
              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
              .childGap = 4,
              .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_BOTTOM},
          }}) {
      for (uint8_t i = 0; i < sizeof(main_menu_buttons) / sizeof(main_menu_buttons[0]); i++) {
        CLAY({.id = CLAY_IDI_LOCAL("Button", i + 1),
              .backgroundColor = state.navigation.hovered == i ? COLOR_CHIPS : COLOR_MULT,
              .layout = {.padding = CLAY_PADDING_ALL(4)}}) {
          CLAY_TEXT(main_menu_buttons[i], WHITE_TEXT_CONFIG);
        }
      }
    }
  }
}

void render_select_deck() {
  CLAY({.layout = {
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 8,
        }}) {
    CLAY({.layout = {.sizing = {CLAY_SIZING_FIXED(256), CLAY_SIZING_FIXED(64)}}, .image = {.imageData = state.logo}}) {}

    CLAY({.layout = {.sizing = {CLAY_SIZING_PERCENT(0.5), CLAY_SIZING_FIT(0)},
                     .padding = CLAY_PADDING_ALL(8),
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .childGap = 8},
          .backgroundColor = COLOR_CARD_BG}) {
      CLAY({.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)}, .childGap = 4}}) {
        CustomElementData *deck_icon = frame_arena_allocate(sizeof(CustomElementData));
        *deck_icon = (CustomElementData){.type = CUSTOM_ELEMENT_DECK, .deck = state.navigation.hovered};
        CLAY({.custom = deck_icon,
              .layout = {.sizing = {CLAY_SIZING_FIXED(CARD_WIDTH), CLAY_SIZING_FIXED(CARD_HEIGHT)}}}) {}

        CLAY({.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                         .layoutDirection = CLAY_TOP_TO_BOTTOM,
                         .childGap = 4}}) {
          Deck deck = get_current_section() == NAVIGATION_SELECT_DECK ? state.navigation.hovered
                                                                      : state.prev_navigation.hovered;
          Clay_String deck_name;
          append_clay_string(&deck_name, "%s", get_deck_name(deck));

          Clay_String deck_description;
          append_clay_string(&deck_description, "%s", get_deck_description(deck));

          CLAY_TEXT(deck_name, WHITE_TEXT_CONFIG);
          CLAY_TEXT(deck_description, WHITE_TEXT_CONFIG);
        }
      }
    }
  }
}

void render_credits() {
  CLAY({.layout = {
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
        }}) {
    CLAY({.layout = {.sizing = {CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0)},
                     .padding = CLAY_PADDING_ALL(16),
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .childGap = 8},
          .backgroundColor = COLOR_CARD_BG}) {
      CLAY({.layout = {.padding = {.bottom = 8}}}) {
        CLAY_TEXT(CLAY_STRING("Following software libraries and assets are utilized in the creation of this game:"),
                  WHITE_TEXT_CONFIG);
      }

      CLAY_TEXT(CLAY_STRING("c-vector by Evan Teran is licensed under MIT"), WHITE_TEXT_CONFIG);
      CLAY_TEXT(CLAY_STRING("Clay by Nic Barker is licensed under zlib"), WHITE_TEXT_CONFIG);
      CLAY_TEXT(CLAY_STRING("Poker cards asset pack by IvoryRed is licensed under CC-BY-4.0 / Changed placement of "
                            "sprites, removed backgrounds, added custom elements"),
                WHITE_TEXT_CONFIG);
      CLAY_TEXT(CLAY_STRING("Pixel Bitmap Fonts by frostyfreeze is licensed under CC0"), WHITE_TEXT_CONFIG);
      CLAY_TEXT(CLAY_STRING("Logo by Grzybson4 is licensed under CC-BY-4.0"), WHITE_TEXT_CONFIG);
    }
  }
}

void render_card_atlas_sprite(Vector2 *sprite_index, Rect *dst) {
  float angle = 3.0f * sinf(state.time * 0.75f - dst->x / SCREEN_WIDTH * M_PI * 3);
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

  Vector2 seal = {.x = 5 + card->seal - 1, .y = 1};
  if (card->seal != SEAL_NONE) render_card_atlas_sprite(&seal, dst);
}

void render_joker(Joker *joker, Rect *dst) {
  Vector2 src = {.x = 9, .y = 1};
  if (joker->id == 6) src.y += 2;

  render_card_atlas_sprite(&src, dst);
}

void render_consumable(Consumable *consumable, Rect *dst) {
  Vector2 index = {.x = 4, .y = 0};
  switch (consumable->type) {
    case CONSUMABLE_PLANET:
      index.y = 5;
      break;
    case CONSUMABLE_TAROT:
      index.y = 1;
      break;

    case CONSUMABLE_SPECTRAL:
      index.y = 7;
      break;
  }

  render_card_atlas_sprite(&index, dst);
}

void render_voucher(Voucher voucher, Rect *dst) { render_card_atlas_sprite(&(Vector2){9, 7}, dst); }

void render_booster_pack(BoosterPackItem *booster_pack, Rect *dst) {
  render_card_atlas_sprite(&(Vector2){.x = 4, .y = 3}, dst);
}

void render_deck(Deck deck, Rect *dst) { render_card_atlas_sprite(&(Vector2){.x = 3, .y = 3}, dst); }

void render_tooltip(Clay_String *title, Clay_String *description, float y_offset,
                    Clay_FloatingAttachPoints *attach_points) {
  CLAY({.id = CLAY_ID("Tooltip"),
        .floating = {
            .attachTo = CLAY_ATTACH_TO_PARENT,
            .zIndex = 10,
            .offset = {.y = y_offset},
            .attachPoints = *attach_points,
        }}) {
    CLAY({.backgroundColor = COLOR_CARD_BG,
          .layout = {
              .padding = CLAY_PADDING_ALL(4),
              .childGap = 2,
              .sizing = {CLAY_SIZING_GROW(0, 100), CLAY_SIZING_GROW(0)},
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
          }}) {
      CLAY_TEXT(*title, WHITE_TEXT_CONFIG);
      CLAY_TEXT(*description, WHITE_TEXT_CONFIG);
    }
  }
}

void render_spread_items(NavigationSection section, Clay_String parent_id) {
  uint8_t item_count = get_nav_section_size(section);
  if (item_count == 0) return;
  size_t items_width = item_count * CARD_WIDTH;

  for (uint8_t i = 0; i < item_count; i++) {
    float parent_width = Clay_GetElementData(CLAY_SID(parent_id)).boundingBox.width - 2 * SECTION_PADDING;
    CustomElementData *element = frame_arena_allocate(sizeof(CustomElementData));
    *element = create_spread_item_element(section, i);

    float x_offset = 0;
    if (item_count == 1)
      x_offset = (float)(parent_width - CARD_WIDTH) / 2;
    else if (items_width <= parent_width)
      x_offset = (float)(parent_width - items_width) / (item_count + 1) * (i + 1) + CARD_WIDTH * i;
    else
      x_offset = i * (float)(parent_width - CARD_WIDTH) / (item_count - 1);

    x_offset += SECTION_PADDING;

    float y_offset = 0;
    if (section == NAVIGATION_HAND && state.game.hand.cards[i].selected == 1) y_offset = -40;

    uint8_t is_hovered = state.navigation.hovered == i && get_current_section() == section;

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
        uint8_t is_shop = section == NAVIGATION_SHOP_ITEMS || section == NAVIGATION_SHOP_BOOSTER_PACKS ||
                          section == NAVIGATION_SHOP_VOUCHER;
        if (is_hovered) {
          Clay_String name;
          Clay_String description;
          get_nav_item_tooltip_content(&name, &description, section);

          float y_offset = -4;
          Clay_FloatingAttachPoints attach_points = {.parent = CLAY_ATTACH_POINT_CENTER_TOP,
                                                     .element = CLAY_ATTACH_POINT_CENTER_BOTTOM};

          if (section == NAVIGATION_JOKERS || section == NAVIGATION_CONSUMABLES) {
            y_offset *= -1;
            attach_points.parent = CLAY_ATTACH_POINT_CENTER_BOTTOM;
            attach_points.element = CLAY_ATTACH_POINT_CENTER_TOP;
          }

          render_tooltip(&name, &description, y_offset, &attach_points);
        }

        if (!is_shop) continue;

        CLAY({.id = CLAY_ID_LOCAL("Price"),
              .floating = {.attachTo = CLAY_ATTACH_TO_PARENT,
                           .offset = {.y = CHAR_HEIGHT},
                           .zIndex = is_hovered ? 10 : 1,
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
                               : section == NAVIGATION_SHOP_VOUCHER ? get_voucher_price(state.game.shop.vouchers[i])
                                                                    : get_shop_item_price(&state.game.shop.items[i]));
            CLAY_TEXT(price, WHITE_TEXT_CONFIG);
          }
        }
      }
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
              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CARD_HEIGHT + 2 * SECTION_PADDING)},
              .childGap = 8,
              .layoutDirection = CLAY_LEFT_TO_RIGHT,
          }}) {
      CLAY({.id = CLAY_ID("Hand"),
            .backgroundColor = COLOR_SECTION_BG,
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
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CARD_HEIGHT + 2 * SECTION_PADDING)},
            .childGap = 4,
        }}) {
    CLAY({.id = CLAY_ID("Jokers"),
          .backgroundColor = COLOR_SECTION_BG,
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
          .backgroundColor = COLOR_SECTION_BG,
          .layout = {
              .sizing = {CLAY_SIZING_PERCENT(0.3), CLAY_SIZING_GROW(0)},
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
    .backgroundColor = COLOR_CARD_LIGHT_BG,
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
        append_clay_string(&stage, "%s", get_blind_name(state.game.current_blind->type));
      else if (state.stage == STAGE_SELECT_BLIND)
        append_clay_string(&stage, "Choose your next Blind");
      else
        append_clay_string(&stage, "SHOP");

      CLAY_TEXT(stage, CLAY_TEXT_CONFIG({.textAlignment = CLAY_TEXT_ALIGN_CENTER, .textColor = COLOR_WHITE}));
    }

    if (state.stage == STAGE_GAME) {
      CLAY(sidebar_block_config) {
        CLAY_TEXT(CLAY_STRING("Score at least:"),
                  CLAY_TEXT_CONFIG({.textColor = COLOR_WHITE, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        Clay_String required_score;
        append_clay_string(&required_score, "%.0lf",
                           get_required_score(state.game.ante, state.game.current_blind->type));

        CLAY_TEXT(required_score, CLAY_TEXT_CONFIG({.textColor = {255, 63, 52, 255}}));
      }
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
                           get_poker_hand_stats(state.game.selected_hand.hand_union)->level + 1);
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
      append_clay_string(&hands, "%d", state.game.hands.remaining);
      CLAY_TEXT(hands, WHITE_TEXT_CONFIG);
    }

    CLAY(sidebar_block_config) {
      CLAY_TEXT(CLAY_STRING("Discards"), WHITE_TEXT_CONFIG);

      Clay_String discards;
      append_clay_string(&discards, "%d", state.game.discards.remaining);
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

      CLAY({.id = CLAY_ID("ShopBottom"),
            .layout = {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CARD_HEIGHT)},
            }}) {
        CLAY({.id = CLAY_ID("ShopVoucher"),
              .layout = {.sizing = CLAY_SIZING_PERCENT(0.3), CLAY_SIZING_FIXED(CARD_HEIGHT)}}) {}
        render_spread_items(NAVIGATION_SHOP_VOUCHER, CLAY_STRING("ShopVoucher"));

        CLAY({.id = CLAY_ID("ShopBoosterPacks"),
              .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CARD_HEIGHT)}}}) {}
        render_spread_items(NAVIGATION_SHOP_BOOSTER_PACKS, CLAY_STRING("ShopBoosterPacks"));
      }
    }
  }
}

void render_booster_pack_content() {
  CLAY({.id = CLAY_ID("BoosterPack"),
        .layout = {
            .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .childGap = 8,
            .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_BOTTOM},
        }}) {
    if (state.game.booster_pack.item.type == BOOSTER_PACK_ARCANA ||
        state.game.booster_pack.item.type == BOOSTER_PACK_SPECTRAL) {
      CLAY({.id = CLAY_ID("BoosterPackHand"),
            .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(CARD_HEIGHT)}}}) {}
      render_spread_items(NAVIGATION_HAND, CLAY_STRING("BoosterPackHand"));
    }

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
      uint8_t discards = get_discards_money();
      uint8_t blind = get_blind_money(state.game.current_blind->type);
      uint8_t investment_tag = get_investment_tag_money();

      CLAY({.layout = {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)},
                .childAlignment = {CLAY_ALIGN_X_CENTER},
            }}) {
        Clay_String cash_out_text;
        append_clay_string(&cash_out_text, "Cash Out: $%d", interest + hands + discards + blind + investment_tag);
        CLAY_TEXT(cash_out_text, CLAY_TEXT_CONFIG({.textColor = COLOR_MONEY}));
      }

      CLAY({.layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(1)}}, .backgroundColor = COLOR_WHITE}) {}

      CLAY({.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM}}) {
        if (blind != 0) {
          Clay_String blind_money;
          append_clay_string(&blind_money, "Blind: $%d", blind);
          CLAY_TEXT(blind_money, WHITE_TEXT_CONFIG);
        }

        if (hands != 0) {
          Clay_String hands_money;
          append_clay_string(&hands_money, "Hands: $%d", hands);
          CLAY_TEXT(hands_money, WHITE_TEXT_CONFIG);
        }

        if (discards != 0) {
          Clay_String discards_money;
          append_clay_string(&discards_money, "Discards: $%d", discards);
          CLAY_TEXT(discards_money, WHITE_TEXT_CONFIG);
        }

        if (interest != 0) {
          Clay_String interest_money;
          append_clay_string(&interest_money, "Interest: $%d", interest);
          CLAY_TEXT(interest_money, WHITE_TEXT_CONFIG);
        }

        if (investment_tag != 0) {
          Clay_String investment_tag_money;
          append_clay_string(&investment_tag_money, "Investment Tag: $%d", investment_tag);
          CLAY_TEXT(investment_tag_money, WHITE_TEXT_CONFIG);
        }
      }
    }
  }
}

void render_blind_element(uint8_t blind_index) {
  Blind *blind = &state.game.blinds[blind_index];
  uint8_t is_current_blind = blind == state.game.current_blind;

  CLAY({.id = CLAY_IDI_LOCAL("Blind", blind_index),
        .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_PERCENT(is_current_blind ? 1.0f : 0.85f)},
                   .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_TOP},
                   .layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .childGap = 4,
                   .padding = CLAY_PADDING_ALL(8)},
        .backgroundColor = COLOR_CARD_BG}) {
    CLAY({.layout = {.sizing = {CLAY_SIZING_GROW(0)},
                     .padding = {.top = 4, .bottom = 4},
                     .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},
          .backgroundColor =
              is_current_blind ? state.navigation.hovered == 0 ? COLOR_CHIPS : COLOR_MONEY : COLOR_CARD_LIGHT_BG}) {
      CLAY_TEXT(is_current_blind                   ? CLAY_STRING("Select")
                : !blind->is_active                ? CLAY_STRING("Skipped")
                : blind < state.game.current_blind ? CLAY_STRING("Defeated")
                                                   : CLAY_STRING("Upcoming"),
                WHITE_TEXT_CONFIG);
    }

    Clay_String blind_name;
    append_clay_string(&blind_name, "%s", get_blind_name(blind->type));
    CLAY_TEXT(blind_name, WHITE_TEXT_CONFIG);

    Clay_String score;
    append_clay_string(&score, "Score at least:\n%.0lf", get_required_score(state.game.ante, blind->type));
    CLAY_TEXT(score, WHITE_TEXT_CONFIG);

    Clay_String money;
    append_clay_string(&money, "Reward: $%d", get_blind_money(blind->type));
    CLAY_TEXT(money, WHITE_TEXT_CONFIG);

    if (blind->type > BLIND_BIG) continue;

    Clay_String tag_name;
    append_clay_string(&tag_name, "%s", get_tag_name(blind->tag));
    Clay_String tag_description;
    append_clay_string(&tag_description, "%s", get_tag_description(blind->tag));

    CLAY_TEXT(CLAY_STRING("or"), WHITE_TEXT_CONFIG);
    CLAY({.layout = {.sizing = {CLAY_SIZING_GROW(0)},
                     .padding = {.top = 4, .bottom = 4},
                     .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}},
          .backgroundColor = is_current_blind && state.navigation.hovered == 1 ? COLOR_CHIPS : COLOR_MULT}) {
      CLAY_TEXT(CLAY_STRING("Skip Blind"), WHITE_TEXT_CONFIG);

      if (is_current_blind && state.navigation.hovered == 1)
        render_tooltip(&tag_name, &tag_description, -4,
                       &(Clay_FloatingAttachPoints){.parent = CLAY_ATTACH_POINT_CENTER_TOP,
                                                    .element = CLAY_ATTACH_POINT_CENTER_BOTTOM});
    }

    CLAY_TEXT(tag_name, WHITE_TEXT_CONFIG);
  }
}

void render_select_blind() {
  CLAY({.id = CLAY_ID("SelectBlind"),
        .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                   .childGap = 16,
                   .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_BOTTOM},
                   .padding = {.top = 16}}}) {
    render_blind_element(0);
    render_blind_element(1);
    render_blind_element(2);
  }
}

void render_game_over() { CLAY_TEXT(CLAY_STRING("You've lost:("), WHITE_TEXT_CONFIG); }

const Clay_ElementDeclaration overlay_bg_config =
    (Clay_ElementDeclaration){.floating = {.zIndex = 10, .attachTo = CLAY_ATTACH_TO_ROOT},
                              .layout =
                                  {
                                      .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                                      .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                                  },
                              .backgroundColor = {0, 0, 0, 150}};

const Clay_String overlay_menu_buttons[] = {CLAY_STRING("Continue"), CLAY_STRING("Poker hands"), CLAY_STRING("Restart"),
                                            CLAY_STRING("Go to main menu")};

void render_overlay_menu() {
  CLAY(overlay_bg_config) {
    CLAY({.layout = {
              .sizing = {CLAY_SIZING_FIT(0), CLAY_SIZING_FIT(0)},
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
              .childGap = 4,
          }}) {
      for (uint8_t i = 0; i < sizeof(overlay_menu_buttons) / sizeof(overlay_menu_buttons[0]); i++) {
        CLAY({.id = CLAY_IDI_LOCAL("Button", i + 1),
              .backgroundColor = state.navigation.hovered == i ? COLOR_CHIPS : COLOR_MULT,
              .layout = {
                  .childAlignment = {CLAY_ALIGN_X_CENTER},
                  .padding = CLAY_PADDING_ALL(4),
                  .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)},
              }}) {
          CLAY_TEXT(overlay_menu_buttons[i], WHITE_TEXT_CONFIG);
        }
      }
    }
  }
}

void render_overlay_select_stake() {
  CLAY(overlay_bg_config) {
    CLAY({.layout = {.sizing = {CLAY_SIZING_PERCENT(0.5), CLAY_SIZING_FIXED(64)},
                     .padding = CLAY_PADDING_ALL(8),
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .childGap = 4},
          .backgroundColor = COLOR_CARD_BG_ALPHA(200)}) {
      Clay_String stake_name;
      append_clay_string(&stake_name, "%s", get_stake_name(state.navigation.hovered));

      Clay_String stake_description;
      append_clay_string(&stake_description, "%s", get_stake_description(state.navigation.hovered));

      CLAY_TEXT(stake_name, WHITE_TEXT_CONFIG);
      CLAY_TEXT(stake_description, WHITE_TEXT_CONFIG);
    }
  }
}

void render_overlay_poker_hands() {
  CLAY(overlay_bg_config) {
    CLAY({.id = CLAY_ID_LOCAL("Wrapper"),
          .backgroundColor = COLOR_CARD_BG_ALPHA(200),
          .layout = {
              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)},
              .padding = CLAY_PADDING_ALL(4),
              .childGap = 4,
              .layoutDirection = CLAY_TOP_TO_BOTTOM,
          }}) {
      for (uint8_t i = 0; i < 12; i++) {
        CLAY({.id = CLAY_IDI_LOCAL("PokerHand", i + 1),
              .backgroundColor = {255, 255, 255, 30},
              .layout = {
                  .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIT(0)},
                  .childGap = 4,
              }}) {
          CLAY({.id = CLAY_ID_LOCAL("Level"),
                .backgroundColor = COLOR_WHITE,
                .layout = {.padding = CLAY_PADDING_ALL(2),
                           .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                           .sizing = {CLAY_SIZING_FIXED(7 * CHAR_WIDTH), CLAY_SIZING_FIT(0)}}}) {
            Clay_String level;
            append_clay_string(&level, "lvl.%d", state.game.poker_hands[i].level + 1);
            CLAY_TEXT(level, CLAY_TEXT_CONFIG({.textColor = COLOR_BLACK}));
          }

          CLAY({.id = CLAY_ID_LOCAL("Name"),
                .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                           .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}}}) {
            const char *name_chars = get_poker_hand_name(1 << i);
            Clay_String name = {.chars = name_chars, .length = strlen(name_chars)};
            CLAY_TEXT(name, WHITE_TEXT_CONFIG);
          }

          CLAY({.id = CLAY_ID_LOCAL("Score"),
                .backgroundColor = COLOR_CARD_LIGHT_BG,
                .layout = {
                    .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
                    .padding = {.left = 2, .right = 2},
                    .sizing = {CLAY_SIZING_FIT(0), CLAY_SIZING_GROW(0)},
                }}) {
            ScorePair score = get_poker_hand_total_score(1 << i);

            CLAY({.id = CLAY_ID_LOCAL("Chips"),
                  .backgroundColor = COLOR_CHIPS,
                  .layout = {
                      .sizing = {CLAY_SIZING_FIXED(4 * CHAR_WIDTH + 4)},
                      .childAlignment = {CLAY_ALIGN_X_RIGHT, CLAY_ALIGN_Y_CENTER},
                      .padding = {.left = 2, .right = 2},
                  }}) {
              Clay_String chips;
              append_clay_string(&chips, "%d", score.chips);
              CLAY_TEXT(chips, WHITE_TEXT_CONFIG);
            }

            CLAY({.layout = {
                      .sizing = {CLAY_SIZING_FIT(0), CLAY_SIZING_GROW(0)},
                      .padding = {.left = 2, .right = 2},
                      .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                  }}) {
              CLAY_TEXT(CLAY_STRING("x"), WHITE_TEXT_CONFIG);
            }

            CLAY({.id = CLAY_ID_LOCAL("Mult"),
                  .backgroundColor = COLOR_MULT,
                  .layout = {
                      .sizing = {CLAY_SIZING_FIXED(4 * CHAR_WIDTH + 4)},
                      .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER},
                      .padding = {.left = 2, .right = 2},
                  }}) {
              Clay_String mult;
              append_clay_string(&mult, "%.0lf", score.mult);
              CLAY_TEXT(mult, WHITE_TEXT_CONFIG);
            }
          }

          CLAY({.id = CLAY_ID_LOCAL("Played"),
                .layout = {.childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                           .sizing = {CLAY_SIZING_FIT(0), CLAY_SIZING_GROW(0)}}}) {
            CLAY_TEXT(CLAY_STRING("#"), WHITE_TEXT_CONFIG);

            CLAY({.layout = {.padding = {.right = 2, .left = 2},
                             .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER},
                             .sizing = {CLAY_SIZING_FIXED(3 * CHAR_WIDTH), CLAY_SIZING_GROW(0)}}}) {
              Clay_String played;
              append_clay_string(&played, "%d", state.game.poker_hands[i].played);
              CLAY_TEXT(played, CLAY_TEXT_CONFIG({.textColor = COLOR_MONEY}));
            }
          }
        }
      }
    }
  }
}

static const int8_t grad2[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
uint8_t PERLIN_PERM[BG_PERIOD];

float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float lerp(float a, float b, float t) { return a + t * (b - a); }
float grad_dot(int hash, float dx, float dy) {
  int8_t *g = (int8_t *)grad2[hash & 7];
  return g[0] * dx + g[1] * dy;
}

float perlin(float x, float y) {
  int xi = (int)floorf(x) % BG_PERIOD;
  int yi = (int)floorf(y) % BG_PERIOD;
  int xi1 = (xi + 1) % BG_PERIOD;
  int yi1 = (yi + 1) % BG_PERIOD;

  float tx = x - floorf(x);
  float ty = y - floorf(y);

  float u = fade(tx);
  float v = fade(ty);

  int aa = PERLIN_PERM[(PERLIN_PERM[xi] + yi) % BG_PERIOD];
  int ab = PERLIN_PERM[(PERLIN_PERM[xi] + yi1) % BG_PERIOD];
  int ba = PERLIN_PERM[(PERLIN_PERM[xi1] + yi) % BG_PERIOD];
  int bb = PERLIN_PERM[(PERLIN_PERM[xi1] + yi1) % BG_PERIOD];

  float a = grad_dot(aa, tx, ty);
  float b = grad_dot(ba, tx - 1, ty);
  float c = grad_dot(ab, tx, ty - 1);
  float d = grad_dot(bb, tx - 1, ty - 1);

  float x1 = lerp(a, b, u);
  float x2 = lerp(c, d, u);
  return lerp(x1, x2, v);
}

void init_background() {
  state.bg = init_texture(BG_NOISE_SIZE, BG_NOISE_SIZE);

  for (int i = 0; i < BG_PERIOD; i++) PERLIN_PERM[i] = i;
  for (int i = BG_PERIOD - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    uint8_t temp = PERLIN_PERM[i];
    PERLIN_PERM[i] = PERLIN_PERM[j];
    PERLIN_PERM[j] = temp;
  }

  float scale = 16.0f;

  for (int y = 0; y < BG_NOISE_SIZE; y++) {
    for (int x = 0; x < BG_NOISE_SIZE; x++) {
      float fx = (float)x / BG_NOISE_SIZE * BG_PERIOD / scale;
      float fy = (float)y / BG_NOISE_SIZE * BG_PERIOD / scale;

      float n = perlin(fx, fy);
      n = (n + 1.0f) * 0.5f;
      if (n < 0.0f) n = 0.0f;
      if (n > 1.0f) n = 1.0f;

      float subtle = 0.3f + n * 0.4f;

      uint8_t r = (uint8_t)(subtle * 52);
      uint8_t g = (uint8_t)(subtle * 130);
      uint8_t b = (uint8_t)(subtle * 255);

      state.bg->data[y * BG_NOISE_SIZE + x] = RGB(r, g, b);
    }
  }
}

void render_background() {
  float x_offest = state.time * SCREEN_WIDTH * 0.0078125f;
  float y_offest = state.time * SCREEN_HEIGHT * 0.0078125f;

  draw_texture(state.bg, &(Rect){.x = x_offest, .y = y_offest, .w = BG_TEXTURE_WIDTH, .h = BG_TEXTURE_HEIGHT},
               &(Rect){.x = 0, .y = 0, .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT}, RGB(255, 255, 255), 0);
}
