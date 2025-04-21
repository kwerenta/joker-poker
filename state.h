#ifndef STATE_H
#define STATE_H

#include <clay.h>
#include <pspctrl.h>

#include "game.h"
#include "system.h"

#define FRAME_ARENA_CAPACITY (5120)

#define MAX_NAV_ROWS 3
#define MAX_NAV_SECTIONS_PER_ROW 2

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
  NAVIGATION_SHOP_ITEMS,
  NAVIGATION_SHOP_BOOSTER_PACKS,
  NAVIGATION_BOOSTER_PACK,
  NAVIGATION_CONSUMABLES,
  NAVIGATION_JOKERS
} NavigationSection;

typedef struct {
  uint8_t count;
  NavigationSection sections[MAX_NAV_SECTIONS_PER_ROW];
} NavigationRow;

typedef struct {
  uint8_t row_count;
  NavigationRow rows[MAX_NAV_ROWS];
} NavigationLayout;

typedef struct {
  uint8_t row;
  uint8_t col;
} NavigationCursor;

typedef struct {
  uint8_t hovered;
  NavigationCursor cursor;
} Navigation;

typedef struct {
  uint8_t data[FRAME_ARENA_CAPACITY];
  size_t offset;
} Arena;

void *frame_arena_allocate(size_t size);
int append_clay_string(Clay_String *dest, const char *format, ...);

uint8_t get_nav_section_size(NavigationSection section);
void move_navigation_cursor(int d_row, int d_col);
NavigationSection get_current_section();

void set_nav_hovered(int8_t new_hovered);
void move_nav_hovered(uint8_t new_position);

void change_stage(Stage stage);

typedef struct {
  Arena frame_arena;

  Texture *cards_atlas;
  Texture *font;

  Controls controls;
  Navigation navigation;

  float delta;
  uint8_t running;

  Stage stage;
  Game game;
} State;

extern State state;

#endif
