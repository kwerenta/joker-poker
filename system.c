#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <stb_image.h>
#include <stdlib.h>

#include "game.h"
#include "gfx.h"
#include "lib/cvector.h"
#include "state.h"
#include "system.h"

void draw_tinted_texture(Texture *texture, Rect *src, Rect *dst,
                         uint32_t color) {
  static TextureVertex vertices[2];

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
  sceGuTexImage(0, texture->width, texture->height, texture->width,
                texture->data);
  sceGuTexFilter(GU_NEAREST, GU_NEAREST);
  sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);

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

void draw_texture(Texture *texture, Rect *src, Rect *dst) {
  draw_tinted_texture(texture, src, dst, 0xFFFFFFFF);
}

Texture *load_texture(const char *filename) {
  Texture *texture = (Texture *)calloc(1, sizeof(Texture));

  texture->data = (uint32_t *)stbi_load(
      filename, &(texture->width), &(texture->height), NULL, STBI_rgb_alpha);

  sceKernelDcacheWritebackInvalidateAll();

  return texture;
}

uint8_t button_pressed(unsigned int button) {
  if ((state.controls.data.Buttons & button) &&
      (state.controls.state & button) == 0) {
    return 1;
  }

  return 0;
}

void handle_controls(uint8_t *hovered) {
  Controls *controls = &state.controls;
  sceCtrlReadBufferPositive(&controls->data, 1);

  switch (state.game.stage) {
  case STAGE_GAME:
    if (button_pressed(PSP_CTRL_RIGHT)) {
      set_hovered_card(hovered, *hovered + 1);
    } else if (button_pressed(PSP_CTRL_LEFT)) {
      set_hovered_card(hovered, *hovered - 1);
    } else if (button_pressed(PSP_CTRL_CROSS)) {
      toggle_card_select(*hovered);
    } else if (button_pressed(PSP_CTRL_SQUARE)) {
      play_hand();
    } else if (button_pressed(PSP_CTRL_CIRCLE)) {
      deselect_all_cards();
    } else if (button_pressed(PSP_CTRL_TRIANGLE)) {
      discard_hand();
    } else if (button_pressed(PSP_CTRL_LTRIGGER)) {
      move_card_in_hand(hovered, *hovered - 1);
    } else if (button_pressed(PSP_CTRL_RTRIGGER)) {
      move_card_in_hand(hovered, *hovered + 1);
    } else if (button_pressed(PSP_CTRL_UP)) {
      sort_hand(0);
    } else if (button_pressed(PSP_CTRL_DOWN)) {
      sort_hand(1);
    }
    break;

  case STAGE_SHOP:
    if (button_pressed(PSP_CTRL_CIRCLE)) {
      state.game.stage = STAGE_GAME;
    } else if (button_pressed(PSP_CTRL_UP)) {
      if (state.game.shop.selected_card > 0)
        state.game.shop.selected_card--;
    } else if (button_pressed(PSP_CTRL_DOWN)) {
      if (state.game.shop.selected_card < cvector_size(state.game.shop.jokers))
        state.game.shop.selected_card++;
    } else if (button_pressed(PSP_CTRL_CROSS)) {
      Joker joker = state.game.shop.jokers[state.game.shop.selected_card];
      if (state.game.money >= joker.base_price &&
          cvector_size(state.game.jokers.cards) < state.game.jokers.size) {
        state.game.money -= joker.base_price;
        cvector_push_back(state.game.jokers.cards, joker);
        cvector_erase(state.game.shop.jokers, state.game.shop.selected_card);

        if (state.game.shop.selected_card >
            cvector_size(state.game.shop.jokers)) {
          state.game.shop.selected_card = cvector_size(state.game.shop.jokers);
        }
      }
    }

  case STAGE_GAME_OVER:
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
  int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11,
                                   0xFA0, 0, 0);
  if (thid >= 0)
    sceKernelStartThread(thid, 0, 0);
  return thid;
}
