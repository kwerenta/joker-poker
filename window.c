#include "window.h"
#include "game.h"
#include "gfx.h"
#include "state.h"

void window_init(SDL_Window **window, SDL_Renderer **renderer) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

  *window =
      SDL_CreateWindow("Joker Poker", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  if (*window == NULL) {
    fprintf(stderr, "Failed to create window: %s", SDL_GetError());
    exit(1);
  }

  *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
  if (*renderer == NULL) {
    fprintf(stderr, "Failed to create renderer: %s", SDL_GetError());
    exit(1);
  }
}

void window_destroy(SDL_Window *window, SDL_Renderer *renderer) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
}

void handle_events(SDL_Event *event, uint8_t *hovered) {
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      state.running = 0;
      break;
    case SDL_CONTROLLERDEVICEADDED:
      SDL_GameControllerOpen(event->cdevice.which);
      break;
    case SDL_KEYDOWN:
      switch (event->key.keysym.sym) {
      case SDLK_ESCAPE:
        state.running = 0;
        break;
      case SDLK_RIGHT:
        if (*hovered < state.game.hand.count - 1) {
          *hovered += 1;
        }
        break;
      case SDLK_LEFT:
        if (*hovered > 0) {
          *hovered -= 1;
        }
        break;
      case SDLK_SPACE:
        toggle_card_select(*hovered);
        printf("Selected hand: %d\n", evaluate_hand());
        break;
      case SDLK_RETURN:
        get_scoring_hand();
        break;
      }
      break;
    case SDL_CONTROLLERBUTTONDOWN:
      switch (event->cbutton.button) {
      case SDL_CONTROLLER_BUTTON_START:
        state.running = 0;
        break;
      case SDL_CONTROLLER_BUTTON_A:
        toggle_card_select(*hovered);
        break;
      case SDL_CONTROLLER_BUTTON_X:
        get_scoring_hand();
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        if (*hovered < state.game.hand.count - 1) {
          *hovered += 1;
        }
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        if (*hovered > 0) {
          *hovered -= 1;
        }
        break;
      }
      break;
    }
  }
}

void render(uint8_t hovered) {
  SDL_RenderClear(state.renderer);

  draw_hand(hovered);

  SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
  SDL_RenderPresent(state.renderer);
}
