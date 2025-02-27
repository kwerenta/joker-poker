#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

#define BUFFER_WIDTH (512)
#define BUFFER_HEIGHT SCREEN_HEIGHT

typedef struct {
  float x, y, w, h;
} Rect;

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
void draw_texture(Texture *texture, Rect *src, Rect *dst);

void handle_controls(uint8_t *hovered);

int setup_callbacks();
void init_gu(void **fbp0, void **fbp1, char list[]);
void end_gu();

void start_frame(char list[]);
void end_frame();

#endif
