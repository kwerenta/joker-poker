#include <pspkernel.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CLAY_IMPLEMENTATION
#include <clay.h>

#include "debug.h"
#include "game.h"
#include "gfx.h"
#include "renderer.h"
#include "state.h"
#include "system.h"

PSP_MODULE_INFO("Joker Poker", 0, 0, 10);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

static char __attribute__((aligned(16))) list[262144];

State state;

void init() {
  log_init();

  srand(time(NULL));

  setup_callbacks();

  init_gu(list);
  renderer_init();

  state.cards_atlas = load_texture("res/cards.png");
  state.font = load_texture("res/font.png");
  state.bg = init_texture(128, 128);

  init_sine_tab();

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  game_init();

  state.delta = 0;
  state.running = 1;

  log_message(LOG_INFO, "Application has been initialized.");
}

void destroy() {
  game_destroy();
  end_gu();

  stbi_image_free(state.cards_atlas->data);
  free(state.cards_atlas);

  stbi_image_free(state.font->data);
  free(state.font);

  free(state.bg);

  log_message(LOG_INFO, "Application has been destroyed.");
}

int main(int argc, char *argv[]) {
  init();
  uint64_t last_time = sceKernelGetSystemTimeWide();

  log_message(LOG_INFO, "Starting main loop...");

  while (state.running) {
    handle_controls();

    uint64_t curr_time = sceKernelGetSystemTimeWide();

    state.delta = (curr_time - last_time) / 1000000.0f;
    state.time += state.delta;

    last_time = curr_time;

    state.frame_arena.offset = 0;

    start_frame(list);

    render_background();

    Clay_BeginLayout();

    CLAY({.id = CLAY_ID("Container"), .layout = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}}}) {
      if (state.stage != STAGE_GAME_OVER) render_sidebar();

      CLAY({.id = CLAY_ID("Content"),
            .layout = {
                .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .padding = {.top = 8, .left = 8, .right = 8, .bottom = 0},
            }}) {
        if (state.stage != STAGE_GAME_OVER) render_topbar();

        switch (state.stage) {
          case STAGE_GAME:
            render_hand();
            break;
          case STAGE_CASH_OUT:
            render_cash_out();
            break;
          case STAGE_SHOP:
            render_shop();
            break;
          case STAGE_GAME_OVER:
            render_game_over();
            break;
          case STAGE_BOOSTER_PACK:
            render_booster_pack_content();
            break;
        }
      }
    }

    switch (state.overlay) {
      case OVERLAY_NONE:
        break;
      case OVERLAY_MENU:
        render_overlay_menu();
        break;
      case OVERLAY_POKER_HANDS:
        render_overlay_poker_hands();
        break;
    }

#ifdef DEBUG_BUILD
    CLAY({.floating = {
              .attachTo = CLAY_ATTACH_TO_ROOT,
              .attachPoints = {.parent = CLAY_ATTACH_POINT_RIGHT_TOP, .element = CLAY_ATTACH_POINT_RIGHT_TOP}}}) {
      Clay_String fps_counter;
      append_clay_string(&fps_counter, "%.2f FPS", 1 / state.delta);
      CLAY_TEXT(fps_counter, WHITE_TEXT_CONFIG);
    }
#endif

    Clay_RenderCommandArray render_commands = Clay_EndLayout();
    execute_render_commands(render_commands);

    flush_render_batch();

    end_frame();
  }

  destroy();
  log_shutdown();

  return 0;
}
