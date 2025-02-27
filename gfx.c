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

  uint8_t xOffset = 0;
  uint8_t yOffest = 0;

  for (; *text;) {
    if (*text == ' ') {
      dst.x += CHAR_WIDTH;
      text++;
      continue;
    }

    xOffset = 0;
    yOffest = 0;

    if (*text >= 'A' && *text <= 'Z') {
      xOffset = *text - 'A';
    } else if (*text >= 'a' && *text <= 'z') {
      xOffset = *text - 'a';
      yOffest = 2;
    } else if (*text >= '0' && *text <= '9') {
      xOffset = *text - '0';
      yOffest = 4;
    } else if (*text >= '!' && *text <= '/') {
      xOffset = *text - '!';
      yOffest = 5;
    } else if (*text >= ':' && *text <= '@') {
      xOffset = *text - ':';
      yOffest = 7;
    } else {
      switch (*text) {
      case '[':
        xOffset = 0;
        break;
      case ']':
        xOffset = 1;
        break;
      case '{':
        xOffset = 2;
        break;
      case '}':
        xOffset = 3;
        break;
      }
      yOffest = 8;
    }

    src.x = (xOffset % 13) * CHAR_WIDTH;
    src.y = (floor(xOffset / 13.0) + yOffest) * CHAR_HEIGHT;

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
