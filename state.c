#include "state.h"

#include "debug.h"
#include "lib/clay.h"

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
