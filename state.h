#ifndef STATE_H
#define STATE_H

#include <pspctrl.h>

#include "game.h"
#include "lib/clay.h"
#include "system.h"

#define FRAME_ARENA_CAPACITY (5120)

typedef enum {
  STAGE_GAME,
  STAGE_CASH_OUT,
  STAGE_SHOP,
  STAGE_BOOSTER_PACK,
  STAGE_GAME_OVER,
} Stage;

typedef struct {
  SceCtrlData data;
  unsigned int state;
} Controls;

typedef enum {
  NAVIGATION_HAND,
  NAVIGATION_SHOP,
  NAVIGATION_BOOSTER_PACK,
  NAVIGATION_CONSUMABLES,
  NAVIGATION_JOKERS
} NavigationSection;

typedef struct {
  uint8_t hovered;
  NavigationSection section;
} Navigation;

typedef struct {
  uint8_t data[FRAME_ARENA_CAPACITY];
  size_t offset;
} Arena;

void *frame_arena_allocate(size_t size);
int append_clay_string(Clay_String *dest, const char *format, ...);

void change_nav_section(NavigationSection section);

void set_nav_hovered(uint8_t new_hovered);
void move_nav_hovered(uint8_t new_position);

void change_stage(Stage stage);

typedef struct {
  Arena frame_arena;

  Texture *cards_atlas;
  Texture *font;

  Controls controls;
  Navigation navigation;

  double delta;
  uint8_t running;

  Stage stage;
  Game game;
} State;

extern State state;

#endif
