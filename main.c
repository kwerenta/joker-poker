#include <pspkernel.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CLAY_IMPLEMENTATION
#include <clay.h>

#define LAY_IMPLEMENTATION
#include "debug.h"
#include "game.h"
#include "gfx.h"
#include "lib/layout.h"
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

  init_background();

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
#ifdef DEBUG_BUILD
  float frame_time = 0.0f;
#endif

  log_message(LOG_INFO, "Starting main loop...");

  lay_context ctx;
  lay_init_context(&ctx);
  lay_reserve_items_capacity(&ctx, 512);

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
      append_clay_string(&fps_counter, "%.2f FPS [%.2f ms]", 1 / state.delta, frame_time);
      CLAY_TEXT(fps_counter, WHITE_TEXT_CONFIG);
    }
#endif

    Clay_RenderCommandArray render_commands = Clay_EndLayout();
    execute_render_commands(render_commands);

    lay_id root = lay_item(&ctx);
    lay_set_size_xy(&ctx, root, SCREEN_WIDTH, SCREEN_HEIGHT);
    lay_set_contain(&ctx, root, LAY_ROW);

    lay_id sidebar = lay_item(&ctx);
    lay_insert(&ctx, root, sidebar);
    lay_set_size_xy(&ctx, sidebar, SIDEBAR_WIDTH, 0);
    lay_set_behave(&ctx, sidebar, LAY_VFILL);
    lay_set_contain(&ctx, sidebar, LAY_COLUMN);

    lay_id game_view = lay_item(&ctx);
    lay_insert(&ctx, root, game_view);
    lay_set_behave(&ctx, game_view, LAY_HFILL | LAY_VFILL);

    lay_id overlay = lay_item(&ctx);
    lay_set_size_xy(&ctx, overlay, SCREEN_WIDTH, SCREEN_HEIGHT);
    lay_set_contain(&ctx, overlay, LAY_COLUMN | LAY_MIDDLE);

    lay_id overlay_content = lay_item(&ctx);
    lay_insert(&ctx, overlay, overlay_content);
    lay_set_contain(&ctx, overlay_content, LAY_COLUMN);
    lay_set_behave(&ctx, overlay_content, LAY_HFILL);
    lay_set_margins_ltrb(&ctx, overlay_content, 8, 8, 8, 8);

    lay_id items[12] = {0};
    items[0] = lay_item(&ctx);
    lay_insert(&ctx, overlay_content, items[0]);

    for (uint8_t i = 0; i < 12; i++) {
      lay_set_size_xy(&ctx, items[i], 0, CHAR_HEIGHT + 4);
      lay_set_contain(&ctx, items[i], LAY_ROW | LAY_START);
      lay_set_behave(&ctx, items[i], LAY_HFILL);

      lay_id level = lay_item(&ctx);
      lay_insert(&ctx, items[i], level);
      lay_set_size_xy(&ctx, level, CHAR_WIDTH * 7, 0);
      lay_set_behave(&ctx, level, LAY_VFILL);

      lay_id name = lay_item(&ctx);
      lay_append(&ctx, level, name);
      lay_set_behave(&ctx, name, LAY_VFILL | LAY_HFILL);
      lay_set_contain(&ctx, name, LAY_MIDDLE);

      if (i == 11) {
        lay_set_margins_ltrb(&ctx, items[i], 4, 4, 4, 4);
        continue;
      }

      lay_set_margins_ltrb(&ctx, items[i], 4, 4, 4, 0);
      items[i + 1] = lay_item(&ctx);
      lay_append(&ctx, items[i], items[i + 1]);
    }

    lay_run_context(&ctx);
    lay_run_item(&ctx, overlay);

    lay_vec4 sidebar_rect = lay_get_rect(&ctx, sidebar);
    lay_vec4 overlay_rect = lay_get_rect(&ctx, overlay);
    lay_vec4 overlay_content_rect = lay_get_rect(&ctx, overlay_content);

    draw_rectangle(&(Rect){sidebar_rect[0], sidebar_rect[1], sidebar_rect[2], sidebar_rect[3]}, RGB(30, 39, 46));

    draw_rectangle(&(Rect){overlay_rect[0], overlay_rect[1], overlay_rect[2], overlay_rect[3]}, RGBA(0, 0, 0, 150));
    draw_rectangle(
        &(Rect){overlay_content_rect[0], overlay_content_rect[1], overlay_content_rect[2], overlay_content_rect[3]},
        RGBA(30, 39, 46, 200));

    for (uint8_t i = 0; i < 12; i++) {
      lay_vec4 rect = lay_get_rect(&ctx, items[i]);
      draw_rectangle(&(Rect){rect[0], rect[1], rect[2], rect[3]}, RGBA(255, 255, 255, 30));

      rect = lay_get_rect(&ctx, lay_first_child(&ctx, items[i]));
      draw_rectangle(&(Rect){rect[0], rect[1], rect[2], rect[3]}, RGB(255, 255, 255));
      draw_text("lvl. 1", &(Vector2){rect[0] + 2, rect[1] + 2}, 0xFF000000);
    }

    lay_reset_context(&ctx);

#ifdef DEBUG_BUILD
    frame_time = (sceKernelGetSystemTimeWide() - curr_time) / 1000.0f;
#endif

    end_frame();
  }

  lay_destroy_context(&ctx);
  destroy();
  log_shutdown();

  return 0;
}
