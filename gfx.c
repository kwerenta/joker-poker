#include "gfx.h"
#include "SDL_surface.h"
#include "game.h"
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

void render_selected_poker_hand() {
  SDL_Color fg_color = {255, 255, 255, 255};
  SDL_Color bg_color = {0, 0, 0, 255};

  SDL_Surface *surface;
  SDL_Texture *texture;

  SDL_Rect hand_rect = {.x = 16, .y = 16};
  SDL_Rect hand_score_rect = {.x = 16, .y = hand_rect.y + 48 + 8};

  char buffer[64];

  if (state.game.selected_hand.count != 0) {
    surface = TTF_RenderUTF8_LCD(
        state.font, get_poker_hand_name(state.game.selected_hand.poker_hand),
        bg_color, fg_color);

    texture = SDL_CreateTextureFromSurface(state.renderer, surface);

    hand_rect.w = surface->w;
    hand_rect.h = surface->h;

    SDL_FreeSurface(surface);
    SDL_RenderCopy(state.renderer, texture, NULL, &hand_rect);
    SDL_DestroyTexture(texture);

    snprintf(buffer, 64, "%llu x %llu", state.game.selected_hand.mult,
             state.game.selected_hand.chips);
    surface = TTF_RenderUTF8_LCD(state.font, buffer, bg_color, fg_color);
    texture = SDL_CreateTextureFromSurface(state.renderer, surface);
    hand_score_rect.w = surface->w;
    hand_score_rect.h = surface->h;

    SDL_FreeSurface(surface);
    SDL_RenderCopy(state.renderer, texture, NULL, &hand_score_rect);
    SDL_DestroyTexture(texture);
  }

  SDL_Rect score_rect = {.x = 16, .y = hand_score_rect.y + 48 + 8};

  snprintf(buffer, 64, "Score: %llu", state.game.score);
  surface = TTF_RenderUTF8_LCD(state.font, buffer, bg_color, fg_color);
  texture = SDL_CreateTextureFromSurface(state.renderer, surface);
  score_rect.w = surface->w;
  score_rect.h = surface->h;

  SDL_FreeSurface(surface);
  SDL_RenderCopy(state.renderer, texture, NULL, &score_rect);
  SDL_DestroyTexture(texture);
}
