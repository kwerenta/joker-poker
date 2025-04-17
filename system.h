#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

#define BUFFER_WIDTH (512)
#define BUFFER_HEIGHT SCREEN_HEIGHT

#define RGB(r, g, b) RGBA(r, g, b, 255)
#define RGBA(r, g, b, a) ((a << 24) | (b << 16) | (g << 8) | r)

typedef struct {
  float x, y, w, h;
} Rect;

typedef struct {
  float x, y;
} Vector2;

typedef struct {
  uint16_t u, v;
  int16_t x, y, z;
} Vertex;

typedef struct {
  float u, v;
  uint32_t color;
  float x, y, z;
} TextureVertex;

typedef struct {
  int width, height;
  uint32_t *data;
} Texture;

Texture *load_texture(const char *filename);
void draw_rectangle(Rect *rect, uint32_t color);
void draw_texture(Texture *texture, Rect *src, Rect *dst, float angle);
void draw_texture_ex(Texture *texture, Rect *src, Rect *dst, uint32_t color, float angle);
Vector2 draw_text(const char *text, const Vector2 *pos, uint32_t color);
Vector2 draw_text_len(const char *text, uint32_t len, const Vector2 *pos, uint32_t color);

void handle_controls();

int setup_callbacks();
void init_gu(void **fbp0, void **fbp1, char list[]);
void end_gu();

void start_frame(char list[]);
void end_frame();

#endif
