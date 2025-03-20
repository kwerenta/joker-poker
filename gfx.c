#include <math.h>
#include <pspgu.h>
#include <stdio.h>

#include "game.h"
#include "gfx.h"
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
  uint32_t white = 0xFFFFFFFF;
  uint32_t block_bg = RGB(72, 84, 96);

  Rect bg = {.x = 0, .y = 0, .w = SIDEBAR_WIDTH, .h = SCREEN_HEIGHT};
  draw_rectangle(&bg, RGB(30, 39, 46));

  Rect blind = {.x = SIDEBAR_GAP, .y = SIDEBAR_GAP, .w = bg.w - 2 * SIDEBAR_GAP, .h = 14};
  draw_rectangle(&blind, RGB(255, 168, 1));

  snprintf(buffer, 64, "Blind %d", state.game.blind + 1);
  uint8_t len = strlen(buffer);
  draw_text(buffer, &(Vector2){.x = blind.x + blind.w / 2.0 - (len / 2.0) * CHAR_WIDTH, .y = blind.y + 2}, white);

  Rect required_score = {.x = blind.x, .y = blind.y + blind.h + SIDEBAR_GAP, .w = blind.w, .h = 26};
  draw_rectangle(&required_score, block_bg);
  const char *score_at_least = "Score at least:";
  len = strlen(score_at_least);
  Vector2 prev_pos = draw_text(
      score_at_least,
      &(Vector2){.x = required_score.x + required_score.w / 2.0 - (len / 2.0) * CHAR_WIDTH, .y = required_score.y + 2},
      white);

  snprintf(buffer, 64, "%.0f", get_required_score(state.game.ante, state.game.blind));
  len = strlen(buffer);
  draw_text(buffer,
            &(Vector2){.x = required_score.x + required_score.w / 2.0 - (len / 2.0) * CHAR_WIDTH,
                       .y = prev_pos.y + CHAR_HEIGHT + 2},
            RGB(255, 63, 52));

  Rect score = {.x = blind.x, .y = required_score.y + required_score.h + SIDEBAR_GAP, .w = blind.w, .h = 26};
  draw_rectangle(&score, block_bg);

  snprintf(buffer, 64, "Round");
  len = strlen(buffer);
  draw_text(buffer, &(Vector2){.x = score.x + 2, .y = score.y + 2}, white);
  snprintf(buffer, 64, "score");
  len = strlen(buffer);
  prev_pos = draw_text(buffer, &(Vector2){.x = score.x + 2, .y = score.y + CHAR_HEIGHT + 4}, white);

  snprintf(buffer, 64, "%.0lf", state.game.score);
  len = strlen(buffer);
  draw_text(
      buffer,
      &(Vector2){.x = prev_pos.x + (score.w - 5 * CHAR_WIDTH) / 2.0 - (len / 2.0) * CHAR_WIDTH, .y = score.y + 6.5},
      RGB(5, 196, 107));

  Rect hand = {.x = blind.x, .y = score.y + score.h + SIDEBAR_GAP, .w = blind.w, .h = 32};
  draw_rectangle(&hand, block_bg);

  Rect chips = {.x = hand.x + SIDEBAR_GAP, .y = hand.y + SIDEBAR_GAP + CHAR_HEIGHT, .w = hand.w / 2.0 - 9, .h = 14};
  draw_rectangle(&chips, RGB(15, 188, 249));

  prev_pos = draw_text("x", &(Vector2){.x = chips.x + chips.w + 2, .y = chips.y + 2}, white);

  Rect mult = {.x = prev_pos.x + 2, .y = chips.y, .w = chips.w, .h = chips.h};
  draw_rectangle(&mult, RGB(255, 63, 52));

  if (state.game.selected_hand.count != 0) {
    snprintf(buffer, 64, "%s (%d)", get_poker_hand_name(state.game.selected_hand.poker_hand),
             state.game.poker_hands[state.game.selected_hand.poker_hand] + 1);
    len = strlen(buffer);
    draw_text(buffer, &(Vector2){.x = hand.x + hand.w / 2.0 - (len / 2.0) * CHAR_WIDTH, .y = hand.y + 2}, white);

    snprintf(buffer, 64, "%d", state.game.selected_hand.scoring.chips);
    len = strlen(buffer);
    draw_text(buffer, &(Vector2){.x = chips.x + chips.w - 2 - len * CHAR_WIDTH, .y = chips.y + 2}, white);

    snprintf(buffer, 64, "%.0lf", state.game.selected_hand.scoring.mult);
    len = strlen(buffer);
    draw_text(buffer, &(Vector2){.x = mult.x + 2, .y = mult.y + 2}, white);
  }

  Rect hands = {.x = blind.x, .y = hand.y + hand.h + SIDEBAR_GAP, .w = blind.w, .h = 26};
  draw_rectangle(&hands, block_bg);
  draw_text("Hands", &(Vector2){.x = hands.x + hands.w / 2.0 - 2.5 * CHAR_WIDTH, .y = hands.y + 2}, white);
  snprintf(buffer, 64, "%d", state.game.hands);
  len = strlen(buffer);
  draw_text(buffer, &(Vector2){.x = hands.x + hands.w / 2.0 - (len / 2.0) * CHAR_WIDTH, .y = hands.y + 14}, white);

  Rect discards = {.x = blind.x, .y = hands.y + hands.h + SIDEBAR_GAP, .w = blind.w, .h = 26};
  draw_rectangle(&discards, block_bg);
  draw_text("Discards", &(Vector2){.x = discards.x + discards.w / 2.0 - 4 * CHAR_WIDTH, .y = discards.y + 2}, white);
  snprintf(buffer, 64, "%d", state.game.discards);
  len = strlen(buffer);
  draw_text(buffer, &(Vector2){.x = discards.x + discards.w / 2.0 - (len / 2.0) * CHAR_WIDTH, .y = discards.y + 14},
            white);

  Rect money = {.x = blind.x, .y = discards.y + discards.h + SIDEBAR_GAP, .w = blind.w, .h = 14};
  draw_rectangle(&money, block_bg);
  snprintf(buffer, 64, "$%d", state.game.money);
  len = strlen(buffer);
  draw_text(buffer, &(Vector2){.x = money.x + money.w / 2.0 - len / 2.0 * CHAR_WIDTH, .y = money.y + 2}, white);

  Rect ante = {.x = blind.x, .y = money.y + money.h + SIDEBAR_GAP, .w = bg.w / 2.0 - 5, .h = 26};
  draw_rectangle(&ante, block_bg);
  draw_text("Ante", &(Vector2){.x = ante.x + ante.w / 2.0 - 2 * CHAR_WIDTH, .y = ante.y + 2}, white);
  snprintf(buffer, 64, "%d/8", state.game.ante);
  len = strlen(buffer);
  draw_text(buffer, &(Vector2){.x = ante.x + ante.w / 2.0 - (len / 2.0) * CHAR_WIDTH, .y = ante.y + 14}, white);

  Rect round = {.x = ante.x + ante.w + 2, .y = ante.y, .w = ante.w, .h = ante.h};
  draw_rectangle(&round, block_bg);
  draw_text("Round", &(Vector2){.x = round.x + round.w / 2.0 - 2.5 * CHAR_WIDTH, .y = round.y + 2}, white);
  snprintf(buffer, 64, "%d", state.game.round);
  len = strlen(buffer);
  draw_text(buffer, &(Vector2){.x = round.x + round.w / 2.0 - (len / 2.0) * CHAR_WIDTH, .y = round.y + 14}, white);

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
