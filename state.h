#ifndef STATE_H
#define STATE_H

#include "game.h"
#include "system.h"

typedef struct {
  Texture *cards_atlas;
  double delta;
  uint8_t running;

  Game game;
} State;

extern State state;

#endif
