#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

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

Texture *loadTexture(const char *filename);
void drawTexture(Texture *texture, Rect *src, Rect *dst);

void handle_controls(uint8_t *hovered);

#endif
