#ifndef STATE_H
#define STATE_H

#include <pspctrl.h>

#include "game.h"
#include "system.h"

typedef struct {
  SceCtrlData data;
  unsigned int state;
} Controls;

typedef struct {
  Texture *cards_atlas;
  Texture *font;

  Controls controls;

  double delta;
  uint8_t running;

  Game game;
} State;

extern State state;

#endif
