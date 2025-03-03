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

char list[0x20000] __attribute__((aligned(64)));

void *fbp0;
void *fbp1;

State state;

void init() {
  srand(time(NULL));

  setup_callbacks();
  init_gu(&fbp0, &fbp1, list);

  state.cards_atlas = load_texture("res/cards.png");
  state.font = load_texture("res/font.png");

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  game_init();

  state.delta = 0;
  state.running = 1;
}

void destroy() {
  game_destroy();
  end_gu();

  stbi_image_free(state.cards_atlas->data);
  free(state.cards_atlas);

  stbi_image_free(state.font->data);
  free(state.font);
}

int main(int argc, char *argv[]) {
  init();

  uint8_t hovered = 0;

  uint32_t last_tick = 0;
  while (state.running) {
    handle_controls(&hovered);

    // uint32_t curr_tick = SDL_GetTicks();
    // state.delta = (curr_tick - last_tick) / 1000.0;
    // last_tick = curr_tick;

    start_frame(list);

    switch (state.stage) {
    case STAGE_GAME:
      render_hand(hovered);
      render_sidebar();
      break;
    case STAGE_SHOP:
      render_shop();
      break;
    case STAGE_GAME_OVER:
      render_game_over();
      break;
    }

    end_frame();
  }

  destroy();

  return 0;
}
