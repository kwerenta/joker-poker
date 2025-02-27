#include <pspgu.h>
#include <pspkernel.h>
#include <stb_image.h>
#include <stdlib.h>

#include "system.h"

void drawTexture(Texture *texture, Rect *src, Rect *dst) {
  static TextureVertex vertices[2];

  vertices[0].u = src->x;
  vertices[0].v = src->y;
  vertices[0].color = 0xFFFFFFFF;
  vertices[0].x = dst->x;
  vertices[0].y = dst->y;
  vertices[0].z = 0.0f;

  vertices[1].u = src->x + src->w;
  vertices[1].v = src->y + src->h;
  vertices[1].color = 0xFFFFFFFF;
  vertices[1].x = dst->x + dst->w;
  vertices[1].y = dst->y + dst->h;
  vertices[1].z = 0.0f;

  sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
  sceGuTexImage(0, texture->width, texture->height, texture->width,
                texture->data);
  sceGuTexFilter(GU_LINEAR, GU_LINEAR);
  sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);

  sceGuEnable(GU_TEXTURE_2D);
  sceGuEnable(GU_BLEND);

  sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
  sceGuDrawArray(GU_SPRITES,
                 GU_COLOR_8888 | GU_TEXTURE_32BITF | GU_VERTEX_32BITF |
                     GU_TRANSFORM_2D,
                 2, 0, vertices);

  sceGuDisable(GU_BLEND);
  sceGuDisable(GU_TEXTURE_2D);
}

Texture *loadTexture(const char *filename) {
  Texture *texture = (Texture *)calloc(1, sizeof(Texture));

  texture->data = (uint32_t *)stbi_load(
      filename, &(texture->width), &(texture->height), NULL, STBI_rgb_alpha);

  sceKernelDcacheWritebackInvalidateAll();

  return texture;
}
