// #define __PSP__

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdint.h>
#include <stdlib.h>

#include "game.h"
#include "state.h"
#include "window.h"

State state;

void init() {
  srand(time(NULL));

  window_init(&state.window, &state.renderer);
  game_init();

  state.delta = 0;
  state.running = 1;

  state.cards_atlas = IMG_LoadTexture(state.renderer, "../res/cards.png");
  state.font = TTF_OpenFont("../res/VT323-Regular.ttf", 64);
}

void destroy() {
  game_destroy();

  TTF_CloseFont(state.font);
  SDL_DestroyTexture(state.cards_atlas);

  window_destroy(state.window, state.renderer);
}

int main(int argc, char *argv[]) {
  init();

  uint8_t hovered = 0;

  SDL_Event event;
  uint32_t last_tick = 0;
  while (state.running) {
    handle_events(&event, &hovered);

    uint32_t curr_tick = SDL_GetTicks();
    state.delta = (curr_tick - last_tick) / 1000.0;
    last_tick = curr_tick;

    render(hovered);
  }

  destroy();

  return 0;
}
