#include <SDL.h>
#include <SDL_image.h>

// #define __PSP__

#ifdef __PSP__

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#else

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#endif

#define CARD_WIDTH 48
#define CARD_HEIGHT 64

SDL_Texture *cards_atlas;

typedef enum { SUIT_HEARTS, SUIT_DIAMONDS, SUIT_SPADES, SUIT_CLUBS } Suit;
typedef enum {
  RANK_ACE,
  RANK_TWO,
  RANK_THREE,
  RANK_FOUR,
  RANK_FIVE,
  RANK_SIX,
  RANK_SEVEN,
  RANK_EIGHT,
  RANK_NINE,
  RANK_TEN,
  RANK_JACK,
  RANK_QUEEN,
  RANK_KING
} Rank;

void draw_card(SDL_Renderer *renderer, Suit suit, Rank rank, SDL_Rect *dst) {
  SDL_Rect src = {.x = rank * CARD_WIDTH,
                  .y = suit * CARD_HEIGHT,
                  .w = CARD_WIDTH,
                  .h = CARD_HEIGHT};
  SDL_RenderCopy(renderer, cards_atlas, &src, dst);
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

  SDL_Window *window =
      SDL_CreateWindow("Joker Poker", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Surface *cards_pixels = IMG_Load("../res/cards.png");
  cards_atlas = SDL_CreateTextureFromSurface(renderer, cards_pixels);
  SDL_FreeSurface(cards_pixels);

  SDL_Rect card_dst2 = {SCREEN_WIDTH / 2 - 48, SCREEN_HEIGHT / 2 - 64, 96, 128};
  SDL_Rect card_dst1 = card_dst2;
  card_dst1.x -= 100;
  SDL_Rect card_dst3 = card_dst2;
  card_dst3.x += 100;

  int running = 1;
  SDL_Event event;
  uint32_t last_tick = 0;
  double delta = 0;
  while (running) {
    // Process input
    if (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        // End the loop if the programs is being closed
        running = 0;
        break;
      case SDL_CONTROLLERDEVICEADDED:
        // Connect a controller when it is connected
        SDL_GameControllerOpen(event.cdevice.which);
        break;
      case SDL_CONTROLLERBUTTONDOWN:
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
          // Close the program if start is pressed
          running = 0;
        }
        break;
      }
    }

    uint32_t curr_tick = SDL_GetTicks();
    delta = (curr_tick - last_tick) / 1000.0;
    last_tick = curr_tick;

    // Clear the screen
    SDL_RenderClear(renderer);

    // Draw a card
    draw_card(renderer, SUIT_DIAMONDS, RANK_JACK, &card_dst1);
    draw_card(renderer, SUIT_SPADES, RANK_ACE, &card_dst2);
    draw_card(renderer, SUIT_CLUBS, RANK_KING, &card_dst3);

    // Draw everything on a white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderPresent(renderer);
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
