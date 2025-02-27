#include <stdint.h>
#include <time.h>

#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspkernel.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "game.h"
#include "gfx.h"
#include "state.h"
#include "system.h"

PSP_MODULE_INFO("joker-poker", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

#define BUFFER_WIDTH (512)
#define BUFFER_HEIGHT SCREEN_HEIGHT

char list[0x20000] __attribute__((aligned(64)));

void *fbp0;
void *fbp1;

State state;

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

void initGu() {
  sceGuInit();

  fbp0 = guGetStaticVramBuffer(BUFFER_WIDTH, BUFFER_HEIGHT, GU_PSM_8888);
  fbp1 = guGetStaticVramBuffer(BUFFER_WIDTH, BUFFER_HEIGHT, GU_PSM_8888);

  sceGuStart(GU_DIRECT, list);
  sceGuDrawBuffer(GU_PSM_8888, fbp0, BUFFER_WIDTH);
  sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, fbp1, BUFFER_WIDTH);

  sceGuDepthBuffer(fbp0, 0);
  sceGuDisable(GU_DEPTH_TEST);

  sceGuOffset(2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2));
  sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
  sceGuEnable(GU_SCISSOR_TEST);
  sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  sceGuFinish();
  sceGuDisplay(GU_TRUE);
}

void endGu() {
  sceGuDisplay(GU_FALSE);
  sceGuTerm();
}

void startFrame() {
  sceGuStart(GU_DIRECT, list);
  sceGuClearColor(0xFFFFFFFF);
  sceGuClear(GU_COLOR_BUFFER_BIT);
}

void endFrame() {
  sceGuFinish();
  sceGuSync(0, 0);
  sceDisplayWaitVblankStart();
  sceGuSwapBuffers();
}

void init() {
  srand(time(NULL));

  setup_callbacks();
  initGu();

  state.cards_atlas = loadTexture("res/cards.png");

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  game_init();

  state.delta = 0;
  state.running = 1;
}

void destroy() {
  game_destroy();
  endGu();

  stbi_image_free(state.cards_atlas->data);
  free(state.cards_atlas);
}

int main(int argc, char *argv[]) {
  init();

  uint8_t hovered = 0;

  uint32_t last_tick = 0;
  while (state.running) {
    handle_controls(&hovered);
    // handle_events(&event, &hovered);

    // uint32_t curr_tick = SDL_GetTicks();
    // state.delta = (curr_tick - last_tick) / 1000.0;
    // last_tick = curr_tick;

    startFrame();

    render_hand(hovered);

    endFrame();
  }

  destroy();

  return 0;
}
