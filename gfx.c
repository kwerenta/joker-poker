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

void render_text(const char *text, float x, float y, uint32_t color) {
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
    // Special characters
    else if (*text >= 33 && *text <= 47) {
      index = *text - 33;
      src.y += 5 * CHAR_HEIGHT;
    }
    // Special characters
    else if (*text >= 58 && *text <= 64) {
      index = *text - 58;
      src.y += 7 * CHAR_HEIGHT;
    }
    // Left square bracket
    else if (*text == 91) {
      index = 0;
      src.y += 8 * CHAR_HEIGHT;
    }
    // Right square bracket
    else if (*text == 93) {
      index = 1;
      src.y += 8 * CHAR_HEIGHT;
    }
    // Left curly bracket
    else if (*text == 123) {
      index = 2;
      src.y += 8 * CHAR_HEIGHT;
    }
    // Right curly bracket
    else if (*text == 125) {
      index = 3;
      src.y += 8 * CHAR_HEIGHT;
    }

    src.x = index % 13 * CHAR_WIDTH;
    src.y += floor(index / 13.0) * CHAR_HEIGHT;

    draw_tinted_texture(state.font, &src, &dst, color);
    dst.x += CHAR_WIDTH;
    text++;
  }
}

void render_sidebar() {
  Rect hand_rect = {.x = 16, .y = 16};
  Rect hand_score_rect = {.x = 16, .y = hand_rect.y + CHAR_HEIGHT + 8};

  uint32_t white = 0xFFFFFFFF;
  uint32_t green = 0xFF00FF00;
  uint32_t red = 0xFF0000FF;

  char buffer[64];

  if (state.game.selected_hand.count != 0) {
    render_text(get_poker_hand_name(state.game.selected_hand.poker_hand),
                hand_rect.x, hand_rect.y, white);

    PokerHandScoring score =
        get_poker_hand_base_scoring(state.game.selected_hand.poker_hand);
    snprintf(buffer, 64, "%d x %d", score.chips, score.mult);
    render_text(buffer, hand_score_rect.x, hand_score_rect.y, green);
  }

  Rect score_rect = {.x = 16, .y = hand_score_rect.y + CHAR_HEIGHT + 8};
  snprintf(buffer, 64, "Score: %.0f/%.0f", state.game.score,
           get_required_score(state.game.ante, state.game.blind));
  render_text(buffer, score_rect.x, score_rect.y, red);

  Rect hands_rect = {.x = 16, .y = score_rect.y + CHAR_HEIGHT + 8};
  snprintf(buffer, 64, "Hands: %d, Discards: %d", state.game.hands,
           state.game.discards);
  render_text(buffer, hands_rect.x, hands_rect.y, white);

  Rect round_rect = {.x = 16, .y = hands_rect.y + CHAR_HEIGHT + 8};
  snprintf(buffer, 64, "Ante: %d/8, Blind: %d/3, Round: %d", state.game.ante,
           state.game.blind + 1, state.game.round);
  render_text(buffer, round_rect.x, round_rect.y, white);
}
