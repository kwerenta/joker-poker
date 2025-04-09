#include "state.h"

#include "debug.h"
#include "lib/clay.h"
#include "lib/cvector.h"

#include <stdarg.h>
#include <stdio.h>

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

void set_nav_hovered(uint8_t new_hovered) {
  uint8_t max_value = 0;

  switch (state.navigation.section) {
  case NAVIGATION_HAND:
    max_value = cvector_size(state.game.hand.cards);
    break;
  case NAVIGATION_SHOP:
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
  }

  if (new_hovered >= max_value || new_hovered < 0)
    return;

  state.navigation.hovered = new_hovered;
}

void change_stage(Stage stage) {
  state.stage = stage;

  switch (stage) {
  case STAGE_GAME_OVER:
  case STAGE_GAME:
    change_nav_section(NAVIGATION_HAND);
    break;

  case STAGE_SHOP:
    change_nav_section(NAVIGATION_SHOP);
    break;

  case STAGE_BOOSTER_PACK:
    change_nav_section(NAVIGATION_BOOSTER_PACK);
    break;
  }
}
