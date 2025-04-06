#include <stdint.h>
#include <time.h>

#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspkernel.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CLAY_IMPLEMENTATION
#include "lib/clay.h"
#include "renderer.h"

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
  renderer_init();

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

    state.frame_arena.offset = 0;

    start_frame(list);

    Clay_BeginLayout();

    CLAY({.id = CLAY_ID("Container"), .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}}}) {

      if (state.stage != STAGE_GAME_OVER)
        render_sidebar();

      switch (state.stage) {
      case STAGE_GAME:
        render_hand(hovered);
        break;
      case STAGE_SHOP:
        render_shop();
        break;
      case STAGE_GAME_OVER:
        render_game_over();
        break;
      case STAGE_BOOSTER_PACK:
        render_booster_pack();
      }
    }

    Clay_RenderCommandArray render_commands = Clay_EndLayout();
    execute_render_commands(render_commands);

    end_frame();
  }

  destroy();

  return 0;
}
