#include <SDL.h>

#ifdef __PSP__

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#else

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

#endif

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

  SDL_Window *window =
      SDL_CreateWindow("window", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Rect square = {216, 96, 34, 64};

  int running = 1;
  SDL_Event event;
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

    // Clear the screen
    SDL_RenderClear(renderer);

    // Draw a red square
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &square);

    // Draw everything on a white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderPresent(renderer);
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
