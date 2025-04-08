#ifndef DEBUG_H
#define DEBUG_H

typedef enum { LOG_INFO, LOG_WARNING, LOG_ERROR } LogLevel;

#ifdef DEBUG_BUILD
// Only define functions if in debug mode
void log_init(void);
void log_shutdown(void);
void log_message(LogLevel level, const char *format, ...);
#else
// Empty macros for release builds (compiler will optimize these out)
#define log_init()
#define log_shutdown()
#define log_message(level, format, ...)
#endif

#endif
