#include "window.h"
#include "SDL_gamecontroller.h"
#include "game.h"
#include "gfx.h"
#include "state.h"
#include <SDL_image.h>
#include <SDL_ttf.h>

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

  if (IMG_Init(IMG_INIT_PNG) < 0) {
    fprintf(stderr, "Failed to initialize SDL2_image: %s", SDL_GetError());
    exit(1);
  }

  if (TTF_Init() < 0) {
    fprintf(stderr, "Failed to initialize SDL2_ttf : %s", SDL_GetError());
    exit(1);
  }
}

void window_destroy(SDL_Window *window, SDL_Renderer *renderer) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  IMG_Quit();

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
        set_hovered_card(hovered, *hovered + 1);
        break;
      case SDLK_LEFT:
        set_hovered_card(hovered, *hovered - 1);
        break;
      case SDLK_LEFTBRACKET:
        move_card_in_hand(hovered, *hovered - 1);
        break;
      case SDLK_RIGHTBRACKET:
        move_card_in_hand(hovered, *hovered + 1);
        break;
      case SDLK_BACKSPACE:
        deselect_all_cards();
        break;
      case SDLK_q:
        discard_hand();
        break;
      case SDLK_r:
        sort_hand(0);
        break;
      case SDLK_s:
        sort_hand(1);
        break;
      case SDLK_SPACE:
        toggle_card_select(*hovered);
        break;
      case SDLK_RETURN:
        play_hand();
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
        play_hand();
        break;
      case SDL_CONTROLLER_BUTTON_B:
        deselect_all_cards();
        break;
      case SDL_CONTROLLER_BUTTON_Y:
        discard_hand();
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        set_hovered_card(hovered, *hovered + 1);
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        set_hovered_card(hovered, *hovered - 1);
        break;
      case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        move_card_in_hand(hovered, *hovered - 1);
        break;
      case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        move_card_in_hand(hovered, *hovered + 1);
        break;
      }
      break;
    }
  }
}

void render(uint8_t hovered) {
  SDL_RenderClear(state.renderer);

  render_sidebar();
  render_hand(hovered);

  SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
  SDL_RenderPresent(state.renderer);
}
