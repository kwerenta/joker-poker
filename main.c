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

typedef enum { UI_ELEMENT_TYPE_RECTANGLE, UI_ELEMENT_TYPE_TEXT } UIElementType;

typedef struct {
  uint32_t color;
} UIElementRectangle;

typedef struct {
  uint32_t color;
  uint32_t bg_color;
  char *content;
} UIELementText;

typedef struct {
  UIElementType type;

  union {
    UIElementRectangle rectangle;
    UIELementText text;
  };
} UIElement;
UIElement ui_elements[512] = {0};

void render_ui_node(lay_context *ctx, lay_id node) {
  UIElement *element = &ui_elements[node];

  if (node == LAY_INVALID_ID) return;

  lay_vec4 rect = lay_get_rect(ctx, node);

  if (element->type == UI_ELEMENT_TYPE_TEXT) {
    draw_text(element->text.content, &(Vector2){rect[0], rect[1]}, element->text.color);
    return;
  }

  if (element->type == UI_ELEMENT_TYPE_RECTANGLE) {
    draw_rectangle(&(Rect){rect[0], rect[1], rect[2], rect[3]}, element->rectangle.color);
    return;
  }
}

void traverse_ui_tree(lay_context *ctx, lay_id node) {
  if (node == LAY_INVALID_ID) return;

  render_ui_node(ctx, node);
  traverse_ui_tree(ctx, lay_first_child(ctx, node));
  traverse_ui_tree(ctx, lay_next_sibling(ctx, node));
}

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

  render_overlay_poker_hands();

  Clay_RenderCommandArray render_commands = Clay_EndLayout();

  while (state.running) {
    handle_controls();

    uint64_t curr_time = sceKernelGetSystemTimeWide();

    state.delta = (curr_time - last_time) / 1000000.0f;
    state.time += state.delta;

    last_time = curr_time;

    state.frame_arena.offset = 0;

    start_frame(list);

    render_background();

#ifdef DEBUG_BUILD
    Clay_String fps_counter;
    append_clay_string(&fps_counter, "%.2f FPS [%.2f ms]", 1 / state.delta, frame_time);
    draw_text_len(fps_counter.chars, fps_counter.length, &(Vector2){366, 0}, 0xFFFFFFFF);
#endif

    execute_render_commands(render_commands);

    // lay_id root = lay_item(&ctx);
    // lay_set_size_xy(&ctx, root, SCREEN_WIDTH, SCREEN_HEIGHT);
    // lay_set_contain(&ctx, root, LAY_ROW);

    // lay_id sidebar = lay_item(&ctx);
    // lay_insert(&ctx, root, sidebar);
    // ui_elements[sidebar] = (UIElement){.type = UI_ELEMENT_TYPE_RECTANGLE, .rectangle = {.color = RGB(30, 39, 46)}};
    // lay_set_size_xy(&ctx, sidebar, SIDEBAR_WIDTH, 0);
    // lay_set_behave(&ctx, sidebar, LAY_VFILL);
    // lay_set_contain(&ctx, sidebar, LAY_COLUMN);

    // lay_id game_view = lay_item(&ctx);
    // lay_insert(&ctx, root, game_view);
    // lay_set_behave(&ctx, game_view, LAY_HFILL | LAY_VFILL);

    // lay_id overlay = lay_item(&ctx);
    // ui_elements[overlay] = (UIElement){.type = UI_ELEMENT_TYPE_RECTANGLE, .rectangle = {.color = RGBA(0, 0, 0,
    // 150)}}; lay_set_size_xy(&ctx, overlay, SCREEN_WIDTH, SCREEN_HEIGHT); lay_set_contain(&ctx, overlay, LAY_COLUMN |
    // LAY_MIDDLE);

    // lay_id overlay_content = lay_item(&ctx);
    // lay_insert(&ctx, overlay, overlay_content);
    // ui_elements[overlay_content] =
    //     (UIElement){.type = UI_ELEMENT_TYPE_RECTANGLE, .rectangle = {.color = RGBA(30, 39, 46, 150)}};
    // lay_set_contain(&ctx, overlay_content, LAY_COLUMN);
    // lay_set_behave(&ctx, overlay_content, LAY_HFILL);
    // lay_set_margins_ltrb(&ctx, overlay_content, 8, 8, 8, 8);

    // lay_id items[12] = {0};
    // items[0] = lay_item(&ctx);
    // lay_insert(&ctx, overlay_content, items[0]);

    // for (uint8_t i = 0; i < 12; i++) {
    //   ui_elements[items[i]] =
    //       (UIElement){.type = UI_ELEMENT_TYPE_RECTANGLE, .rectangle = {.color = RGBA(255, 255, 255, 30)}};
    //   lay_set_size_xy(&ctx, items[i], 0, CHAR_HEIGHT + 4);
    //   lay_set_contain(&ctx, items[i], LAY_ROW | LAY_START);
    //   lay_set_behave(&ctx, items[i], LAY_HFILL);

    //   lay_id level = lay_item(&ctx);
    //   ui_elements[level] = (UIElement){.type = UI_ELEMENT_TYPE_RECTANGLE, .rectangle = {.color = RGB(255, 255,
    //   255)}}; lay_insert(&ctx, items[i], level); lay_set_size_xy(&ctx, level, CHAR_WIDTH * 7, 0);
    //   lay_set_behave(&ctx, level, LAY_VFILL);

    //   lay_id level_text = lay_item(&ctx);
    //   ui_elements[level_text] =
    //       (UIElement){.type = UI_ELEMENT_TYPE_TEXT, .text = {.content = "lvl.1", .color = RGB(0, 0, 0)}};
    //   lay_insert(&ctx, level, level_text);
    //   lay_set_margins_ltrb(&ctx, level_text, 2, 2, 2, 2);
    //   lay_set_behave(&ctx, level_text, LAY_LEFT | LAY_TOP);

    //   lay_id name = lay_item(&ctx);
    //   ui_elements[name] =
    //       (UIElement){.type = UI_ELEMENT_TYPE_TEXT, .text = {.content = "Poker hand", .color = RGB(255, 255, 255)}};
    //   lay_append(&ctx, level, name);
    //   lay_set_margins_ltrb(&ctx, name, 2, 2, 2, 2);
    //   lay_set_behave(&ctx, name, LAY_VFILL | LAY_HFILL);

    //   lay_id chips = lay_item(&ctx);
    //   ui_elements[chips] = (UIElement){.type = UI_ELEMENT_TYPE_RECTANGLE, .rectangle = {.color = RGB(15, 188, 249)}};
    //   lay_append(&ctx, name, chips);
    //   lay_set_margins_ltrb(&ctx, chips, 2, 2, 2, 2);
    //   lay_set_size_xy(&ctx, chips, CHAR_WIDTH * 4, 0);
    //   lay_set_behave(&ctx, chips, LAY_VFILL | LAY_RIGHT);

    //   lay_id x = lay_item(&ctx);
    //   ui_elements[x] = (UIElement){.type = UI_ELEMENT_TYPE_TEXT, .text = {.content = "x", .color = RGB(255, 255,
    //   255)}}; lay_append(&ctx, chips, x); lay_set_size_xy(&ctx, x, CHAR_WIDTH, 0); lay_set_behave(&ctx, x, LAY_TOP |
    //   LAY_RIGHT);

    //   lay_id mult = lay_item(&ctx);
    //   ui_elements[mult] = (UIElement){.type = UI_ELEMENT_TYPE_RECTANGLE, .rectangle = {.color = RGB(255, 63, 52)}};
    //   lay_append(&ctx, x, mult);
    //   lay_set_margins_ltrb(&ctx, mult, 2, 2, 2, 2);
    //   lay_set_size_xy(&ctx, mult, CHAR_WIDTH * 4, 0);
    //   lay_set_behave(&ctx, mult, LAY_VFILL | LAY_RIGHT);

    //   lay_id count = lay_item(&ctx);
    //   ui_elements[count] =
    //       (UIElement){.type = UI_ELEMENT_TYPE_TEXT, .text = {.content = "# 1", .color = RGB(255, 255, 255)}};
    //   lay_append(&ctx, mult, count);
    //   lay_set_margins_ltrb(&ctx, count, 2, 2, 2, 2);
    //   lay_set_size_xy(&ctx, count, CHAR_WIDTH * 4, 0);
    //   lay_set_behave(&ctx, count, LAY_VFILL | LAY_TOP | LAY_RIGHT);

    //   if (i == 11) {
    //     lay_set_margins_ltrb(&ctx, items[i], 4, 4, 4, 4);
    //     continue;
    //   }

    //   lay_set_margins_ltrb(&ctx, items[i], 4, 4, 4, 0);
    //   items[i + 1] = lay_item(&ctx);
    //   lay_append(&ctx, items[i], items[i + 1]);
    // }

    // lay_run_context(&ctx);
    // lay_run_item(&ctx, overlay);

    // traverse_ui_tree(&ctx, root);
    // traverse_ui_tree(&ctx, overlay);

    // lay_reset_context(&ctx);

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
