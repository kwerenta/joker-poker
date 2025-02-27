#include <math.h>
#include <pspgu.h>
#include <stdio.h>

#include "game.h"
#include "gfx.h"
#include "state.h"
#include "system.h"

void render_card(Suit suit, Rank rank, Rect *dst) {
  Rect src = {.x = rank * CARD_WIDTH,
              .y = suit * CARD_HEIGHT,
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

void render_text(const char *text, float x, float y) {
  Rect dst = {.x = x, .y = y, .w = 6, .h = 10};
  Rect src = {.x = 0, .y = 0, .w = 6, .h = 10};

  uint8_t index = 0;

  for (; *text;) {
    src.y = 0;

    // Space
    if (*text == 32) {
      dst.x += CHAR_WIDTH;
      text++;
      continue;
    }

    // Capital letters
    if (*text >= 65 && *text <= 90) {
      index = *text - 65;
    }
    // Lowercase letters
    else if (*text >= 97 && *text <= 122) {
      index = *text - 97;
      src.y += 2 * CHAR_HEIGHT;
    }
    // Digits
    else if (*text >= 48 && *text <= 57) {
      index = *text - 48;
      src.y += 4 * CHAR_HEIGHT;
    }
    // Colon
    else if (*text == 58) {
      index = 75;
    }
    // Comma
    else if (*text == 44) {
      index = 81;
    }
    // Slash
    else if (*text == 47) {
      index = 73;
    }

    src.x = index % 13 * CHAR_WIDTH;
    src.y += floor(index / 13.0) * CHAR_HEIGHT;

    draw_texture(state.font, &src, &dst);
    dst.x += CHAR_WIDTH;
    text++;
  }
}

void render_sidebar() {
  Rect hand_rect = {.x = 16, .y = 16};
  Rect hand_score_rect = {.x = 16, .y = hand_rect.y + CHAR_HEIGHT + 8};

  char buffer[64];

  if (state.game.selected_hand.count != 0) {
    render_text(get_poker_hand_name(state.game.selected_hand.poker_hand),
                hand_rect.x, hand_rect.y);

    PokerHandScoring score =
        get_poker_hand_base_scoring(state.game.selected_hand.poker_hand);
    snprintf(buffer, 64, "%llu x %llu", score.chips, score.mult);
    render_text(buffer, hand_score_rect.x, hand_score_rect.y);
  }

  Rect score_rect = {.x = 16, .y = hand_score_rect.y + CHAR_HEIGHT + 8};
  snprintf(buffer, 64, "Score: %.0lf/%.0lf", state.game.score,
           get_required_score(state.game.ante, state.game.blind));
  render_text(buffer, score_rect.x, score_rect.y);

  Rect hands_rect = {.x = 16, .y = score_rect.y + CHAR_HEIGHT + 8};
  snprintf(buffer, 64, "Hands: %d, Discards: %d", state.game.hands,
           state.game.discards);
  render_text(buffer, hands_rect.x, hands_rect.y);

  Rect round_rect = {.x = 16, .y = hands_rect.y + CHAR_HEIGHT + 8};
  snprintf(buffer, 64, "Ante: %d/8, Blind: %d/3, Round: %d", state.game.ante,
           state.game.blind + 1, state.game.round);
  render_text(buffer, round_rect.x, round_rect.y);
}
