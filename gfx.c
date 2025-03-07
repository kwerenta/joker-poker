#include <math.h>
#include <pspgu.h>
#include <stdio.h>

#include "game.h"
#include "gfx.h"
#include "state.h"
#include "system.h"

void render_card(Suit suit, Rank rank, Rect *dst) {
  Rect src = {.x = (rank % 10) * CARD_WIDTH,
              .y = (2 * suit + floor(rank / 10.0)) * CARD_HEIGHT,
              .w = CARD_WIDTH,
              .h = CARD_HEIGHT};
  draw_texture(state.cards_atlas, &src, dst);
}

void render_hand(uint8_t hovered) {
  const Hand *hand = &state.game.hand;

  uint32_t hand_width = cvector_size(hand->cards) * (CARD_WIDTH - 16);
  for (uint8_t i = 0; i < cvector_size(hand->cards); i++) {
    if (hovered == i) {
      continue;
    }

    Rect dst = {.x = SCREEN_WIDTH / 2.0 - hand_width / 2.0 +
                     (CARD_WIDTH - 16) * i,
                .y = SCREEN_HEIGHT - CARD_HEIGHT - 16,
                .w = CARD_WIDTH,
                .h = CARD_HEIGHT};

    if (hand->cards[i].selected == 1) {
      dst.y -= 50;
    }

    render_card(hand->cards[i].suit, hand->cards[i].rank, &dst);
  }

  render_card(hand->cards[hovered].suit, hand->cards[hovered].rank,
              &(Rect){.x = SCREEN_WIDTH / 2.0 - hand_width / 2.0 +
                           (CARD_WIDTH - 16) * hovered - CARD_WIDTH * 0.1,
                      .y = SCREEN_HEIGHT - CARD_HEIGHT - 16 -
                           CARD_HEIGHT * 0.1 -
                           (hand->cards[hovered].selected == 1 ? 50 : 0),
                      .w = CARD_WIDTH * 1.2,
                      .h = CARD_HEIGHT * 1.2});
}

void render_sidebar() {
  Vector2 hand_pos = {.x = 16, .y = 16};
  Vector2 hand_score_pos = {.x = 16, .y = hand_pos.y + CHAR_HEIGHT + 8};

  uint32_t white = 0xFFFFFFFF;
  uint32_t green = 0xFF00FF00;
  uint32_t red = 0xFF0000FF;
  uint32_t blue = 0xFFD07F06;

  char buffer[64];

  if (state.game.selected_hand.count != 0) {
    draw_text(get_poker_hand_name(state.game.selected_hand.poker_hand),
              &hand_pos, white);

    PokerHandScoring score =
        get_poker_hand_total_scoring(state.game.selected_hand.poker_hand);

    snprintf(buffer, 64, "%d", score.chips);
    hand_score_pos = draw_text(buffer, &hand_score_pos, blue);

    snprintf(buffer, 64, " x ");
    hand_score_pos = draw_text(buffer, &hand_score_pos, white);

    snprintf(buffer, 64, "%d", score.mult);
    draw_text(buffer, &hand_score_pos, red);
  }

  Vector2 score_pos = {.x = 16, .y = hand_score_pos.y + CHAR_HEIGHT + 8};
  snprintf(buffer, 64, "Score: %.0f/%.0f", state.game.score,
           get_required_score(state.game.ante, state.game.blind));
  draw_text(buffer, &score_pos, green);

  Vector2 hands_pos = {.x = 16, .y = score_pos.y + CHAR_HEIGHT + 8};
  snprintf(buffer, 64, "Hands: %d, Discards: %d", state.game.hands,
           state.game.discards);
  draw_text(buffer, &hands_pos, white);

  Vector2 round_pos = {.x = 16, .y = hands_pos.y + CHAR_HEIGHT + 8};
  snprintf(buffer, 64, "Ante: %d/8, Blind: %d/3, Round: %d", state.game.ante,
           state.game.blind + 1, state.game.round);
  draw_text(buffer, &round_pos, white);

  snprintf(buffer, 64, "Deck: %zu/%zu", cvector_size(state.game.deck),
           cvector_size(state.game.full_deck));
  draw_text(buffer, &(Vector2){400, 254}, white);
}

void render_shop() {
  char buffer[64];
  Vector2 pos = {10, 10};

  snprintf(buffer, 64, "Money: $%d", state.game.money);
  draw_text(buffer, &pos, 0xFFFFFFFF);

  for (uint8_t i = 0; i < cvector_size(state.game.shop.items); i++) {
    uint32_t color =
        state.game.shop.selected_card == i ? 0xFF00FF00 : 0xFFFFFFFF;
    ShopItem item = state.game.shop.items[i];

    pos.y += 16;

    switch (item.type) {
    case SHOP_ITEM_JOKER:
      draw_text(item.joker.name, &pos, color);

      pos.y += 8;
      draw_text(item.joker.description, &pos, color);
      break;

    case SHOP_ITEM_CARD:
      snprintf(buffer, 64, "Card(Suit=%d, Rank=%d)", item.card.suit,
               item.card.rank);
      draw_text(buffer, &pos, color);
      break;

    case SHOP_ITEM_PLANET:
      draw_text("Pluto Planet", &pos, color);
      break;

    case SHOP_ITEM_BOOSTER_PACK:
      draw_text("Jumbo Standard Pack", &pos, color);
      break;
    }

    pos.y += 8;
    snprintf(buffer, 64, "$%d", item.price);
    draw_text(buffer, &pos, color);
  }
}

void render_booster_pack() {
  char buffer[64];
  Vector2 pos = {10, 10};

  snprintf(buffer, 64, "Money: $%d", state.game.money);
  draw_text(buffer, &pos, 0xFFFFFFFF);

  for (uint8_t i = 0; i < cvector_size(state.game.booster_pack.content); i++) {
    uint32_t color =
        state.game.booster_pack.selected_item == i ? 0xFF00FF00 : 0xFFFFFFFF;
    BoosterPackContent item = state.game.booster_pack.content[i];

    pos.y += 16;

    switch (state.game.booster_pack.item.type) {
    case BOOSTER_PACK_BUFFON:
      draw_text(item.joker.name, &pos, color);

      pos.y += 8;
      draw_text(item.joker.description, &pos, color);
      break;

    case BOOSTER_PACK_STANDARD:
      snprintf(buffer, 64, "Card(Suit=%d, Rank=%d)", item.card.suit,
               item.card.rank);
      draw_text(buffer, &pos, color);
      break;

    case BOOSTER_PACK_CELESTIAL:
      snprintf(buffer, 64, "Planet(%d)", item.planet);
      draw_text(buffer, &pos, color);
      break;
    }
  }
}

void render_game_over() {
  Vector2 pos = {10, 10};
  draw_text("You've lost:(", &pos, 0xFFFFFFFF);
}
