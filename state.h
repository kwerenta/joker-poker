#ifndef STATE_H
#define STATE_H

#include "game.h"
#include <SDL.h>
#include <SDL_ttf.h>

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *cards_atlas;
  TTF_Font *font;
  double delta;
  uint8_t running;

  Game game;
} State;

extern State state;

#endif
