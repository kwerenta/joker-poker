#ifndef STATE_H
#define STATE_H

#include <pspctrl.h>

#include "game.h"
#include "lib/clay.h"
#include "system.h"

#define FRAME_ARENA_CAPACITY (5120)

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
  uint8_t data[FRAME_ARENA_CAPACITY];
  size_t offset;
} Arena;

void *frame_arena_allocate(size_t size);
int append_clay_string(Clay_String *dest, const char *format, ...);

typedef struct {
  Arena frame_arena;

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
