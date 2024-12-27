#ifndef WINDOW_H
#define WINDOW_H

#include "game.h"
#include <SDL.h>

#ifdef __PSP__

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#else

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#endif

void window_init(SDL_Window **window, SDL_Renderer **renderer);
void window_destroy(SDL_Window *window, SDL_Renderer *renderer);

void handle_events(SDL_Event *event, uint8_t *hovered);
void render(uint8_t hovered);

#endif
