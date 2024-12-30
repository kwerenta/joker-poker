#include "gfx.h"
#include "state.h"
#include "window.h"

void render_card(Suit suit, Rank rank, SDL_Rect *dst) {
  SDL_Rect src = {.x = rank * CARD_SPRITE_WIDTH,
                  .y = suit * CARD_SPRITE_HEIGHT,
                  .w = CARD_SPRITE_WIDTH,
                  .h = CARD_SPRITE_HEIGHT};
  SDL_RenderCopy(state.renderer, state.cards_atlas, &src, dst);
}

void render_hand(uint8_t hovered) {
  const Hand *hand = &state.game.hand;

  uint32_t hand_width = cvector_size(hand->cards) * (CARD_WIDTH - 16);
  for (uint8_t i = 0; i < cvector_size(hand->cards); i++) {
    if (hovered == i) {
      continue;
    }

    SDL_Rect dst = {.x = SCREEN_WIDTH / 2 - hand_width / 2 +
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
              &(SDL_Rect){.x = SCREEN_WIDTH / 2.0 - hand_width / 2.0 +
                               (CARD_WIDTH - 16) * hovered - CARD_WIDTH * 0.1,
                          .y = SCREEN_HEIGHT - CARD_HEIGHT - 16 -
                               CARD_HEIGHT * 0.1 -
                               (hand->cards[hovered].selected == 1 ? 50 : 0),
                          .w = CARD_WIDTH * 1.2,
                          .h = CARD_HEIGHT * 1.2});
}
