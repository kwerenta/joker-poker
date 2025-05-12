#include "system.h"

#include <math.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <stb_image.h>
#include <stdlib.h>

#include "game.h"
#include "gfx.h"
#include "state.h"

static RenderBatch render_batch = {.count = 0};

void draw_rectangle(Rect *rect, uint32_t color) { draw_texture(NULL, &(Rect){0, 0, 0, 0}, rect, color, 0); }

void draw_texture(Texture *texture, Rect *src, Rect *dst, uint32_t color, float angle) {
  if (render_batch.texture != texture || render_batch.is_angled != (angle != 0) ||
      render_batch.count >= RENDER_BATCH_SIZE - 6)
    flush_render_batch();

  render_batch.texture = texture;
  render_batch.is_angled = (angle != 0);

  if (angle != 0) {
    float cx = dst->x + dst->w / 2.0f;
    float cy = dst->y + dst->h / 2.0f;

    float rad = angle * (M_PI / 180.0f);
    float cos_a = cosf(rad);
    float sin_a = sinf(rad);

    // Corners of the rectangle before rotation (relative to center)
    float dx[4] = {-dst->w / 2.0f, dst->w / 2.0f, dst->w / 2.0f, -dst->w / 2.0f};
    float dy[4] = {-dst->h / 2.0f, -dst->h / 2.0f, dst->h / 2.0f, dst->h / 2.0f};

    float u[4] = {src->x, src->x + src->w, src->x + src->w, src->x};
    float v[4] = {src->y, src->y, src->y + src->h, src->y + src->h};

    static const uint8_t idx[6] = {0, 1, 2, 0, 2, 3};
    for (int i = 0; i < 6; ++i) {
      int j = idx[i];
      Vertex vertex = {.x = cos_a * dx[j] - sin_a * dy[j] + cx,
                       .y = sin_a * dx[j] + cos_a * dy[j] + cy,
                       .z = 0.0f,
                       .u = u[j],
                       .v = v[j],
                       .color = color};
      render_batch.vertices[render_batch.count++] = vertex;
    }
    return;
  }

  Vertex vertex = {.u = src->x, .v = src->y, .color = color, .x = dst->x, .y = dst->y, .z = 0.0f};
  render_batch.vertices[render_batch.count++] = vertex;

  vertex.u += src->w;
  vertex.v += src->h;
  vertex.x += dst->w;
  vertex.y += dst->h;
  render_batch.vertices[render_batch.count++] = vertex;
}

void flush_render_batch() {
  if (render_batch.count == 0) return;
  Texture *texture = render_batch.texture;

  if (texture != NULL) {
    sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
    sceGuTexImage(0, texture->width, texture->height, texture->width, texture->data);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
    sceGuEnable(GU_TEXTURE_2D);
  }

  sceGuEnable(GU_BLEND);
  sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

  sceGuDrawArray(render_batch.is_angled == 0 ? GU_SPRITES : GU_TRIANGLES,
                 GU_COLOR_8888 | GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D, render_batch.count, 0,
                 render_batch.vertices);

  sceGuDisable(GU_BLEND);
  if (texture != NULL) sceGuDisable(GU_TEXTURE_2D);

  render_batch.count = 0;
}

Texture *load_texture(const char *filename) {
  Texture *texture = (Texture *)calloc(1, sizeof(Texture));

  texture->data = (uint32_t *)stbi_load(filename, &(texture->width), &(texture->height), NULL, STBI_rgb_alpha);

  sceKernelDcacheWritebackInvalidateAll();

  return texture;
}

Texture *init_texture(uint32_t width, uint32_t height) {
  Texture *texture = (Texture *)calloc(1, sizeof(Texture));

  texture->width = width;
  texture->height = height;
  texture->data = guGetStaticVramTexture(width, height, GU_PSM_8888);

  sceKernelDcacheWritebackInvalidateAll();

  return texture;
}

uint8_t button_pressed(unsigned int button) {
  if ((state.controls.data.Buttons & button) && (state.controls.state & button) == 0) {
    return 1;
  }

  return 0;
}

Vector2 draw_text_len(const char *text, uint32_t len, const Vector2 *pos, uint32_t color) {
  Rect dst = {.x = pos->x, .y = pos->y, .w = 6, .h = 10};
  Rect src = {.x = 0, .y = 0, .w = 6, .h = 10};

  uint8_t xOffset = 0;
  uint8_t yOffest = 0;

  for (; len > 0 && *text; len--) {
    if (*text == ' ') {
      dst.x += CHAR_WIDTH;
      text++;
      continue;
    }

    xOffset = 0;
    yOffest = 0;

    if (*text >= 'A' && *text <= 'Z') {
      xOffset = *text - 'A';
    } else if (*text >= 'a' && *text <= 'z') {
      xOffset = *text - 'a';
      yOffest = 2;
    } else if (*text >= '0' && *text <= '9') {
      xOffset = *text - '0';
      yOffest = 4;
    } else if (*text >= '!' && *text <= '/') {
      xOffset = *text - '!';
      yOffest = 5;
    } else if (*text >= ':' && *text <= '@') {
      xOffset = *text - ':';
      yOffest = 7;
    } else {
      switch (*text) {
        case '[':
          xOffset = 0;
          break;
        case ']':
          xOffset = 1;
          break;
        case '{':
          xOffset = 2;
          break;
        case '}':
          xOffset = 3;
          break;
      }
      yOffest = 8;
    }

    src.x = (xOffset % 13) * CHAR_WIDTH;
    src.y = (floor(xOffset / 13.0) + yOffest) * CHAR_HEIGHT;

    draw_texture(state.font, &src, &dst, color, 0);
    dst.x += CHAR_WIDTH;
    text++;
  }

  return (Vector2){.x = dst.x, .y = dst.y};
}

Vector2 draw_text(const char *text, const Vector2 *pos, uint32_t color) {
  return draw_text_len(text, strlen(text), pos, color);
}

uint8_t handle_navigation_controls() {
  NavigationSection section = get_current_section();
  uint8_t hovered = state.navigation.hovered;
  uint8_t section_size = get_nav_section_size(section);
  uint8_t is_horizontal = is_nav_section_horizontal(section);

  if (button_pressed(PSP_CTRL_RIGHT)) {
    if (is_horizontal && hovered < section_size - 1)
      set_nav_hovered(hovered + 1);
    else
      move_nav_cursor(NAVIGATION_RIGHT);

    return 1;
  } else if (button_pressed(PSP_CTRL_LEFT)) {
    if (is_horizontal && hovered > 0)
      set_nav_hovered(hovered - 1);
    else
      move_nav_cursor(NAVIGATION_LEFT);

    return 1;
  } else if (button_pressed(PSP_CTRL_UP)) {
    if (!is_horizontal && hovered > 0)
      set_nav_hovered(hovered - 1);
    else
      move_nav_cursor(NAVIGATION_UP);

    return 1;
  } else if (button_pressed(PSP_CTRL_DOWN)) {
    if (!is_horizontal && hovered < section_size - 1)
      set_nav_hovered(hovered + 1);
    else
      move_nav_cursor(NAVIGATION_DOWN);

    return 1;
  } else if (button_pressed(PSP_CTRL_LTRIGGER)) {
    move_nav_hovered(hovered - 1);
    return 1;
  } else if (button_pressed(PSP_CTRL_RTRIGGER)) {
    move_nav_hovered(hovered + 1);
    return 1;
  } else if (button_pressed(PSP_CTRL_START)) {
    change_overlay(state.overlay == OVERLAY_NONE ? OVERLAY_MENU : OVERLAY_NONE);
    return 1;
  }

  if (section == NAVIGATION_CONSUMABLES) {
    if (button_pressed(PSP_CTRL_CROSS)) {
      use_consumable(NULL);
      return 1;
    }
  }

  if (section == NAVIGATION_OVERLAY_MENU) {
    if (button_pressed(PSP_CTRL_CROSS)) {
      overlay_menu_button_click();
      return 1;
    }
  }

  if (state.overlay != OVERLAY_NONE && state.overlay != OVERLAY_MENU) {
    if (button_pressed(PSP_CTRL_CIRCLE)) {
      change_overlay(OVERLAY_MENU);
      return 1;
    }
  }

  return 0;
}

void handle_controls() {
  Controls *controls = &state.controls;
  sceCtrlReadBufferPositive(&controls->data, 1);

  if (handle_navigation_controls() == 1 || state.overlay != OVERLAY_NONE) {
    state.controls.state = controls->data.Buttons;
    return;
  }

  switch (state.stage) {
    case STAGE_GAME:
      if (button_pressed(PSP_CTRL_CROSS)) {
        toggle_card_select(state.navigation.hovered);
      } else if (button_pressed(PSP_CTRL_SQUARE)) {
        play_hand();
      } else if (button_pressed(PSP_CTRL_CIRCLE)) {
        deselect_all_cards();
      } else if (button_pressed(PSP_CTRL_TRIANGLE)) {
        discard_hand();
      } else if (button_pressed(PSP_CTRL_SELECT)) {
        sort_hand(state.game.sorting_mode == SORTING_BY_SUIT ? SORTING_BY_RANK : SORTING_BY_SUIT);
      }

      break;

    case STAGE_CASH_OUT:
      if (button_pressed(PSP_CTRL_CROSS)) get_cash_out();
      break;

    case STAGE_SHOP:
      if (button_pressed(PSP_CTRL_CIRCLE)) {
        exit_shop();
      } else if (button_pressed(PSP_CTRL_CROSS)) {
        buy_shop_item();
      }
      break;

    case STAGE_BOOSTER_PACK:
      if (button_pressed(PSP_CTRL_CIRCLE)) {
        skip_booster_pack();
      } else if (button_pressed(PSP_CTRL_CROSS)) {
        if (get_current_section() == NAVIGATION_HAND)
          toggle_card_select(state.navigation.hovered);
        else
          select_booster_pack_item();
      }
      break;

    case STAGE_GAME_OVER:
      if (button_pressed(PSP_CTRL_CROSS)) {
        game_destroy();
        game_init();
      }

      break;
  }

  controls->state = controls->data.Buttons;
}

void init_gu(void **fbp0, void **fbp1, char list[]) {
  sceGuInit();

  *fbp0 = guGetStaticVramBuffer(BUFFER_WIDTH, BUFFER_HEIGHT, GU_PSM_8888);
  *fbp1 = guGetStaticVramBuffer(BUFFER_WIDTH, BUFFER_HEIGHT, GU_PSM_8888);

  sceGuStart(GU_DIRECT, list);
  sceGuDrawBuffer(GU_PSM_8888, *fbp0, BUFFER_WIDTH);
  sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, *fbp1, BUFFER_WIDTH);

  sceGuDepthBuffer(*fbp0, 0);
  sceGuDisable(GU_DEPTH_TEST);

  sceGuOffset(2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2));
  sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
  sceGuEnable(GU_SCISSOR_TEST);
  sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  sceGuFinish();
  sceGuDisplay(GU_TRUE);
}

void end_gu() {
  sceGuDisplay(GU_FALSE);
  sceGuTerm();
}

void start_frame(char list[]) {
  sceGuStart(GU_DIRECT, list);

  render_batch.vertices = (Vertex *)sceGuGetMemory(RENDER_BATCH_SIZE * sizeof(Vertex));
  render_batch.count = 0;

  sceGuClearColor(0xFF000000);
  sceGuClear(GU_COLOR_BUFFER_BIT);
}

void end_frame() {
  sceGuFinish();
  sceGuSync(0, 0);
  sceDisplayWaitVblankStart();
  sceGuSwapBuffers();
}

int exit_callback(int arg1, int arg2, void *common) {
  state.running = 0;
  return 0;
}

int callback_thread(SceSize args, void *argp) {
  int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
  sceKernelRegisterExitCallback(cbid);
  sceKernelSleepThreadCB();
  return 0;
}

int setup_callbacks() {
  int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, 0);
  if (thid >= 0) sceKernelStartThread(thid, 0, 0);
  return thid;
}
