#ifndef STATE_H
#define STATE_H

#include <pspctrl.h>

#include "game.h"
#include "system.h"

typedef enum {
  STAGE_GAME,
  STAGE_SHOP,
  STAGE_BOOSTER_PACK,
  STAGE_GAME_OVER,
} Stage;

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

  Stage stage;
  Game game;
} State;

extern State state;

#endif
