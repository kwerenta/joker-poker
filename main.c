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

PSP_MODULE_INFO("Joker Poker", 0, 0, 40);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

static char __attribute__((aligned(16))) list[262144];

State state;

void init() {
  log_init();

  setup_callbacks();

  init_gu(list);
  renderer_init();

  state.cards_atlas = load_texture("res/cards.png");
  state.jokers_atlas1 = load_texture("res/jokers1.png");
  state.jokers_atlas2 = load_texture("res/jokers2.png");
  state.font = load_texture("res/font.png");
  state.logo = load_texture("res/logo.png");

  init_background();

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  state.delta = 0;
  state.running = 1;

  log_message(LOG_INFO, "Application has been initialized.");
}

void destroy() {
  game_destroy();
  end_gu();

  stbi_image_free(state.cards_atlas->data);
  free(state.cards_atlas);

  stbi_image_free(state.jokers_atlas1->data);
  free(state.jokers_atlas1);
  stbi_image_free(state.jokers_atlas2->data);
  free(state.jokers_atlas2);

  stbi_image_free(state.font->data);
  free(state.font);

  stbi_image_free(state.logo->data);
  free(state.logo);

  free(state.bg);

  log_message(LOG_INFO, "Application has been destroyed.");
}

int main(int argc, char *argv[]) {
  init();
  uint64_t last_time = sceKernelGetSystemTimeWide();
#ifdef DEBUG_BUILD
  float frame_time = 0.0f;
#endif

  log_message(LOG_INFO, "Starting main loop...");

  update_render_commands();

  while (state.running) {
    handle_controls();

    uint64_t curr_time = sceKernelGetSystemTimeWide();

    state.delta = (curr_time - last_time) / 1000000.0f;
    state.time += state.delta;
    last_time = curr_time;

    state.frame_arena.offset = 0;

    start_frame(list);

    render_background();

    execute_render_commands(state.render_commands);

#ifdef DEBUG_BUILD
    Clay_String fps_counter;
    append_clay_string(&fps_counter, "%.2f FPS [%.2f ms]", 1 / state.delta, frame_time);
    draw_text_len(fps_counter.chars, fps_counter.length, &(Vector2){360, 0}, 0xFFFFFFFF);

    frame_time = (sceKernelGetSystemTimeWide() - curr_time) / 1000.0f;
#endif

    end_frame();
  }

  destroy();
  log_shutdown();

  return 0;
}
