#ifndef STATE_H
#define STATE_H

#include "game.h"

typedef struct {
  double delta;
  uint8_t running;

  Game game;
} State;

extern State state;

#endif
