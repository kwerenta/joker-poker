#ifndef STATE_H
#define STATE_H

#include <clay.h>
#include <pspctrl.h>

#include "game.h"
#include "system.h"

#define FRAME_ARENA_CAPACITY (5120)

#define MAX_NAV_ROWS 3
#define MAX_NAV_SECTIONS_PER_ROW 2

typedef struct {
  SceCtrlData data;
  unsigned int state;
} Controls;

typedef enum {
  STAGE_MAIN_MENU,
  STAGE_SELECT_DECK,
  STAGE_CREDITS,

  STAGE_GAME,
  STAGE_CASH_OUT,
  STAGE_SHOP,
  STAGE_BOOSTER_PACK,
  STAGE_SELECT_BLIND,
  STAGE_GAME_OVER,
} Stage;

typedef enum { OVERLAY_NONE, OVERLAY_MENU, OVERLAY_SELECT_STAKE, OVERLAY_POKER_HANDS } Overlay;

typedef enum {
  NAVIGATION_NONE,
  NAVIGATION_MAIN_MENU,
  NAVIGATION_SELECT_DECK,
  NAVIGATION_SELECT_STAKE,
  NAVIGATION_SELECT_BLIND,
  NAVIGATION_HAND,
  NAVIGATION_SHOP_ITEMS,
  NAVIGATION_SHOP_VOUCHER,
  NAVIGATION_SHOP_BOOSTER_PACKS,
  NAVIGATION_BOOSTER_PACK,
  NAVIGATION_CONSUMABLES,
  NAVIGATION_JOKERS,
  NAVIGATION_OVERLAY_MENU
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

typedef enum { NAVIGATION_UP, NAVIGATION_DOWN, NAVIGATION_LEFT, NAVIGATION_RIGHT } NavigationDirection;

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
uint8_t is_nav_section_horizontal(NavigationSection section);
void move_nav_cursor(NavigationDirection direction);
NavigationSection get_current_section();

void set_nav_hovered(int8_t new_hovered);
void move_nav_hovered(uint8_t new_position);

void change_stage(Stage stage);
void change_overlay(Overlay overlay);

void overlay_menu_button_click();
void main_menu_button_click();
void select_blind_button_click();

typedef struct {
  Arena frame_arena;
  Clay_RenderCommandArray render_commands;

  Texture *cards_atlas;
  Texture *jokers_atlas1;
  Texture *jokers_atlas2;
  Texture *font;
  Texture *bg;
  Texture *logo;

  Controls controls;

  Stage stage;
  Stage prev_stage;
  Overlay overlay;
  Navigation navigation;
  Navigation prev_navigation;

  float delta;
  float time;
  uint8_t running;

  Game game;
} State;

extern State state;

#endif
