#include "gfx.h"
#include "SDL_render.h"
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

// TODO text surface and texture should only be updated when text has changed,
// as recreating it every frame can affect performance
void render_text(SDL_Rect *dst, char *text, SDL_Color bg, SDL_Color fg) {
  SDL_Surface *surface = TTF_RenderUTF8_LCD(state.font, text, bg, fg);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(state.renderer, surface);

  dst->w = surface->w;
  dst->h = surface->h;

  SDL_FreeSurface(surface);
  SDL_RenderCopy(state.renderer, texture, NULL, dst);
  SDL_DestroyTexture(texture);
}

void render_sidebar() {
  SDL_Color white = {255, 255, 255, 255};
  SDL_Color black = {0, 0, 0, 255};

  SDL_Rect hand_rect = {.x = 16, .y = 16};
  SDL_Rect hand_score_rect = {
      .x = 16, .y = hand_rect.y + TTF_FontHeight(state.font) + 8};

  char buffer[64];

  if (state.game.selected_hand.count != 0) {
    render_text(&hand_rect,
                get_poker_hand_name(state.game.selected_hand.poker_hand), black,
                white);

    snprintf(buffer, 64, "%llu x %llu", state.game.selected_hand.chips,
             state.game.selected_hand.mult);
    render_text(&hand_score_rect, buffer, black, white);
  }

  SDL_Rect score_rect = {
      .x = 16, .y = hand_score_rect.y + TTF_FontHeight(state.font) + 8};
  snprintf(buffer, 64, "Score: %.0lf/%.0lf", state.game.score,
           get_required_score(state.game.ante, state.game.blind));
  render_text(&score_rect, buffer, black, white);

  SDL_Rect hands_rect = {.x = 16,
                         .y = score_rect.y + TTF_FontHeight(state.font) + 8};
  snprintf(buffer, 64, "Hands: %d, Discards: %d", state.game.hands,
           state.game.discards);
  render_text(&hands_rect, buffer, black, white);

  SDL_Rect round_rect = {.x = 16,
                         .y = hands_rect.y + TTF_FontHeight(state.font) + 8};
  snprintf(buffer, 64, "Ante: %d/8, Blind: %d/3, Round: %d", state.game.ante,
           state.game.blind + 1, state.game.round);
  render_text(&round_rect, buffer, black, white);
}
