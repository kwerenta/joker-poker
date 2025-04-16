#include "state.h"

#include <clay.h>
#include <cvector.h>
#include <stdarg.h>
#include <stdio.h>

#include "debug.h"

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

void change_nav_section(NavigationSection section) {
  state.navigation.hovered = 0;
  state.navigation.section = section;
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

void set_nav_hovered(uint8_t new_hovered) {
  uint8_t max_value = get_nav_section_size(state.navigation.section);

  if (new_hovered >= max_value || new_hovered < 0) return;

  state.navigation.hovered = new_hovered;
}

void move_nav_hovered(uint8_t new_position) {
  uint8_t max_position = get_nav_section_size(state.navigation.section);

  if (new_position >= max_position) return;

  uint8_t *hovered = &state.navigation.hovered;

  switch (state.navigation.section) {
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

  switch (stage) {
    case STAGE_GAME:
      change_nav_section(NAVIGATION_HAND);
      break;

    case STAGE_SHOP:
      change_nav_section(NAVIGATION_SHOP_ITEMS);
      break;

    case STAGE_BOOSTER_PACK:
      change_nav_section(NAVIGATION_BOOSTER_PACK);
      break;

    case STAGE_CASH_OUT:
    case STAGE_GAME_OVER:
      break;
  }
}
