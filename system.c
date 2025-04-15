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
#include "system.h"

void draw_rectangle(Rect *rect, uint32_t color) {
  Vertex *vertices = (Vertex *)sceGuGetMemory(2 * sizeof(Vertex));

  vertices[0].x = rect->x;
  vertices[0].y = rect->y;

  vertices[1].x = rect->x + rect->w;
  vertices[1].y = rect->y + rect->h;

  sceGuEnable(GU_BLEND);
  sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

  sceGuColor(color);
  sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);

  sceGuDisable(GU_BLEND);
}

void draw_tinted_texture(Texture *texture, Rect *src, Rect *dst, uint32_t color) {
  TextureVertex *vertices = (TextureVertex *)sceGuGetMemory(2 * sizeof(TextureVertex));

  vertices[0].u = src->x;
  vertices[0].v = src->y;
  vertices[0].color = color;
  vertices[0].x = dst->x;
  vertices[0].y = dst->y;
  vertices[0].z = 0.0f;

  vertices[1].u = src->x + src->w;
  vertices[1].v = src->y + src->h;
  vertices[1].color = color;
  vertices[1].x = dst->x + dst->w;
  vertices[1].y = dst->y + dst->h;
  vertices[1].z = 0.0f;

  sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
  sceGuTexImage(0, texture->width, texture->height, texture->width, texture->data);
  sceGuTexFilter(GU_NEAREST, GU_NEAREST);
  sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);

  sceGuEnable(GU_TEXTURE_2D);
  sceGuEnable(GU_BLEND);

  sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
  sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, vertices);

  sceGuDisable(GU_BLEND);
  sceGuDisable(GU_TEXTURE_2D);
}

void draw_texture(Texture *texture, Rect *src, Rect *dst) { draw_tinted_texture(texture, src, dst, 0xFFFFFFFF); }

Texture *load_texture(const char *filename) {
  Texture *texture = (Texture *)calloc(1, sizeof(Texture));

  texture->data = (uint32_t *)stbi_load(filename, &(texture->width), &(texture->height), NULL, STBI_rgb_alpha);

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

    draw_tinted_texture(state.font, &src, &dst, color);
    dst.x += CHAR_WIDTH;
    text++;
  }

  return (Vector2){.x = dst.x, .y = dst.y};
}

Vector2 draw_text(const char *text, const Vector2 *pos, uint32_t color) {
  return draw_text_len(text, strlen(text), pos, color);
}

void handle_controls() {
  Controls *controls = &state.controls;
  sceCtrlReadBufferPositive(&controls->data, 1);

  switch (state.stage) {
  case STAGE_GAME:
    if (button_pressed(PSP_CTRL_RIGHT)) {
      set_nav_hovered(state.navigation.hovered + 1);
    } else if (button_pressed(PSP_CTRL_LEFT)) {
      set_nav_hovered(state.navigation.hovered - 1);
    } else if (button_pressed(PSP_CTRL_CROSS)) {
      if (state.navigation.section == NAVIGATION_CONSUMABLES)
        use_consumable(NULL);
      else if (state.navigation.section == NAVIGATION_HAND)
        toggle_card_select(state.navigation.hovered);
    } else if (button_pressed(PSP_CTRL_SQUARE)) {
      play_hand();
    } else if (button_pressed(PSP_CTRL_CIRCLE)) {
      deselect_all_cards();
    } else if (button_pressed(PSP_CTRL_TRIANGLE)) {
      discard_hand();
    } else if (button_pressed(PSP_CTRL_LTRIGGER)) {
      move_nav_hovered(state.navigation.hovered - 1);
    } else if (button_pressed(PSP_CTRL_RTRIGGER)) {
      move_nav_hovered(state.navigation.hovered + 1);
    } else if (button_pressed(PSP_CTRL_UP)) {
      sort_hand(0);
    } else if (button_pressed(PSP_CTRL_DOWN)) {
      sort_hand(1);
    } else if (button_pressed(PSP_CTRL_SELECT)) {
      switch (state.navigation.section) {
      case NAVIGATION_HAND:
        change_nav_section(NAVIGATION_JOKERS);
        break;
      case NAVIGATION_JOKERS:
        change_nav_section(NAVIGATION_CONSUMABLES);
        break;
      case NAVIGATION_CONSUMABLES:
        change_nav_section(NAVIGATION_HAND);
        break;

      default:
        break;
      }
    }

    break;

  case STAGE_CASH_OUT:
    if (button_pressed(PSP_CTRL_CROSS))
      get_cash_out();
    break;

  case STAGE_SHOP:
    if (button_pressed(PSP_CTRL_CIRCLE)) {
      exit_shop();
    } else if (button_pressed(PSP_CTRL_UP)) {
      if (state.navigation.section == NAVIGATION_SHOP_BOOSTER_PACKS)
        change_nav_section(NAVIGATION_SHOP_ITEMS);
    } else if (button_pressed(PSP_CTRL_DOWN)) {
      if (state.navigation.section == NAVIGATION_SHOP_ITEMS)
        change_nav_section(NAVIGATION_SHOP_BOOSTER_PACKS);
    } else if (button_pressed(PSP_CTRL_LEFT)) {
      set_nav_hovered(state.navigation.hovered - 1);
    } else if (button_pressed(PSP_CTRL_RIGHT)) {
      set_nav_hovered(state.navigation.hovered + 1);
    } else if (button_pressed(PSP_CTRL_CROSS)) {
      buy_shop_item();
    }

    break;

  case STAGE_BOOSTER_PACK:
    if (button_pressed(PSP_CTRL_CIRCLE)) {
      state.stage = STAGE_SHOP;
    } else if (button_pressed(PSP_CTRL_UP)) {
      set_nav_hovered(state.navigation.hovered - 1);
    } else if (button_pressed(PSP_CTRL_DOWN)) {
      set_nav_hovered(state.navigation.hovered + 1);
    } else if (button_pressed(PSP_CTRL_CROSS)) {
      toggle_booster_pack_item_select();
    } else if (button_pressed(PSP_CTRL_TRIANGLE)) {
      submit_booster_pack();
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
  if (thid >= 0)
    sceKernelStartThread(thid, 0, 0);
  return thid;
}
