#include "state.h"

#include <clay.h>
#include <cvector.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#include "debug.h"

static const NavigationLayout nav_layouts[] = {
    {.row_count = 2,
     .rows =
         {
             {2, {NAVIGATION_JOKERS, NAVIGATION_CONSUMABLES}},
             {1, {NAVIGATION_HAND}},
         }},
    {.row_count = 0},
    {
        .row_count = 3,
        .rows =
            {
                {2, {NAVIGATION_JOKERS, NAVIGATION_CONSUMABLES}},
                {1, {NAVIGATION_SHOP_ITEMS}},
                {1, {NAVIGATION_SHOP_BOOSTER_PACKS}},
            },
    },
    {.row_count = 3,
     .rows =
         {
             {2, {NAVIGATION_JOKERS, NAVIGATION_CONSUMABLES}},
             {1, {NAVIGATION_HAND}},
             {1, {NAVIGATION_BOOSTER_PACK}},
         }},
    {.row_count = 0},
};

int append_clay_string(Clay_String *dest, const char *format, ...) {
  size_t remaining = FRAME_ARENA_CAPACITY - state.frame_arena.offset;
  char *dst = (char *)state.frame_arena.data + state.frame_arena.offset;

  va_list args;
  va_start(args, format);
  int written = vsnprintf(dst, remaining, format, args);
  va_end(args);

  if (written < 0 || (size_t)written >= remaining) {
    log_message(LOG_ERROR, "Frame arena overflow: Failed to append Clay string.");
    written = (remaining > 0) ? remaining - 1 : 0;
  }

  dest->isStaticallyAllocated = 0;
  dest->chars = dst;
  dest->length = written;

  state.frame_arena.offset += written;

  return written;
}

void *frame_arena_allocate(size_t size) {
  // Align offset to 16 bytes as it is required by PSP device
  size_t alignment = 16;
  size_t padding = (alignment - (state.frame_arena.offset % alignment)) % alignment;
  state.frame_arena.offset += padding;

  if (state.frame_arena.offset + size > FRAME_ARENA_CAPACITY) {
    log_message(LOG_ERROR, "Frame arena overflow: Failed to allocate memory.");
    state.running = 0;
    return NULL;
  }

  void *ptr = &state.frame_arena.data[state.frame_arena.offset];
  state.frame_arena.offset += size;
  return ptr;
}

uint8_t calc_proportional_hovered(uint8_t current_count, uint8_t next_count) {
  uint8_t current_hovered = state.navigation.hovered;

  if (current_count <= 1 || next_count <= 1) return 0;
  if (current_count == next_count) return current_hovered < next_count ? current_hovered : next_count - 1;

  float ratio = (float)current_hovered / (float)(current_count - 1);
  int new_hovered = (int)roundf(ratio * (next_count - 1));

  if (new_hovered < 0) new_hovered = 0;
  if (new_hovered >= next_count) new_hovered = next_count - 1;

  return (uint8_t)new_hovered;
}

void move_nav_cursor(NavigationDirection direction) {
  const NavigationLayout *layout = &nav_layouts[state.stage];
  NavigationCursor *cursor = &state.navigation.cursor;
  NavigationSection initial_section = get_current_section();

  int8_t d_row = direction == NAVIGATION_UP ? -1 : direction == NAVIGATION_DOWN ? 1 : 0;
  int8_t d_col = direction == NAVIGATION_LEFT ? -1 : direction == NAVIGATION_RIGHT ? 1 : 0;

  do {
    int new_row = (cursor->row + d_row + layout->row_count) % layout->row_count;

    uint8_t col_count = layout->rows[new_row].count;
    if (col_count == 0) {
      log_message(LOG_WARNING, "Moved navigation cursor to row with 0 columns.");
      return;
    }

    uint8_t curr_column = 0;

    do {
      int new_col = (cursor->col + d_col + col_count) % col_count;
      if (new_col >= col_count) new_col = col_count - 1;

      cursor->row = new_row;
      cursor->col = new_col + (d_col == 0 ? curr_column : 0);
      curr_column++;
    } while (get_nav_section_size(get_current_section()) == 0 && curr_column < col_count);
  } while (get_nav_section_size(get_current_section()) == 0 && initial_section != get_current_section());

  uint8_t initial_section_size = get_nav_section_size(initial_section);
  uint8_t new_section_size = get_nav_section_size(get_current_section());
  if (d_row != 0)
    state.navigation.hovered = calc_proportional_hovered(initial_section_size, new_section_size);
  else
    state.navigation.hovered = state.navigation.hovered == 0 && state.navigation.hovered != initial_section_size - 1
                                   ? new_section_size - 1
                                   : 0;
}

NavigationSection get_current_section() {
  return nav_layouts[state.stage].rows[state.navigation.cursor.row].sections[state.navigation.cursor.col];
}

uint8_t get_nav_section_size(NavigationSection section) {
  uint8_t max_value = 0;

  switch (section) {
    case NAVIGATION_HAND:
      max_value = cvector_size(state.game.hand.cards);
      break;
    case NAVIGATION_SHOP_ITEMS:
      max_value = cvector_size(state.game.shop.items);
      break;
    case NAVIGATION_BOOSTER_PACK:
      max_value = cvector_size(state.game.booster_pack.content);
      break;
    case NAVIGATION_CONSUMABLES:
      max_value = cvector_size(state.game.consumables.items);
      break;
    case NAVIGATION_JOKERS:
      max_value = cvector_size(state.game.jokers.cards);
      break;
    case NAVIGATION_SHOP_BOOSTER_PACKS:
      max_value = cvector_size(state.game.shop.booster_packs);
      break;
  }

  return max_value;
}

void set_nav_hovered(int8_t new_hovered) {
  uint8_t max_value = get_nav_section_size(get_current_section());

  if (new_hovered >= max_value)
    new_hovered = max_value - 1;
  else if (new_hovered < 0)
    new_hovered = 0;

  state.navigation.hovered = new_hovered;
}

void move_nav_hovered(uint8_t new_position) {
  const NavigationSection section = get_current_section();
  uint8_t max_position = get_nav_section_size(section);

  if (new_position >= max_position) return;

  uint8_t *hovered = &state.navigation.hovered;

  switch (section) {
    case NAVIGATION_HAND: {
      Card temp = state.game.hand.cards[*hovered];
      state.game.hand.cards[*hovered] = state.game.hand.cards[new_position];
      state.game.hand.cards[new_position] = temp;
      break;
    }

    case NAVIGATION_JOKERS: {
      Joker temp = state.game.jokers.cards[*hovered];
      state.game.jokers.cards[*hovered] = state.game.jokers.cards[new_position];
      state.game.jokers.cards[new_position] = temp;
      break;
    }

    case NAVIGATION_CONSUMABLES: {
      Consumable temp = state.game.consumables.items[*hovered];
      state.game.consumables.items[*hovered] = state.game.consumables.items[new_position];
      state.game.consumables.items[new_position] = temp;
      break;
    }

    default:
      return;
  }

  *hovered = new_position;
}

void change_stage(Stage stage) {
  state.stage = stage;
  state.navigation.hovered = 0;

  switch (stage) {
    case STAGE_GAME:
      state.navigation.cursor.col = 0;
      state.navigation.cursor.row = 1;
      break;

    case STAGE_SHOP:
      state.navigation.cursor.col = 0;
      state.navigation.cursor.row = 1;
      break;

    case STAGE_BOOSTER_PACK:
      state.navigation.cursor.col = 0;
      state.navigation.cursor.row = 2;
      break;

    case STAGE_CASH_OUT:
    case STAGE_GAME_OVER:
      break;
  }
}
