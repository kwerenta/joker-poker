#include "debug.h"

#ifdef DEBUG_BUILD

#include <pspiofilemgr.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const char *LOG_FILENAME = "ms0:/joker_poker_debug.log";
static const char *LOG_LEVEL_NAMES[] = {"INFO", "WARNING", "ERROR"};

void log_init() {
  SceUID file = sceIoOpen(LOG_FILENAME, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
  if (file >= 0) {
    const char *header = "=== Joker Poker Debug Log ===\n";
    sceIoWrite(file, header, strlen(header));
    sceIoClose(file);
  }
}

void log_shutdown() {
  SceUID file = sceIoOpen(LOG_FILENAME, PSP_O_WRONLY | PSP_O_APPEND, 0777);
  if (file >= 0) {
    const char *footer = "\n=== Log Closed ===\n";
    sceIoWrite(file, footer, strlen(footer));
    sceIoClose(file);
  }
}

void log_message(LogLevel level, const char *format, ...) {
  char buffer[512];
  char time_str[32];

  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  size_t len = strlen(buffer);
  if (len > 0 && buffer[len - 1] != '\n') {
    if (len < sizeof(buffer) - 2) {
      buffer[len] = '\n';
      buffer[len + 1] = '\0';
    }
  }

  SceUID file = sceIoOpen(LOG_FILENAME, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
  if (file >= 0) {
    sceIoWrite(file, buffer, strlen(buffer));
    sceIoClose(file);
  }
}

#endif
