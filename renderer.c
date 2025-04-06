#include "renderer.h"
#include <math.h>
#include <stdio.h>

#include "gfx.h"
#include "lib/clay.h"
#include "state.h"
#include "system.h"

#define CLAY_COLOR_TO_PSP(color)                                                                                       \
  RGBA((uint8_t)roundf(color.r), (uint8_t)roundf(color.g), (uint8_t)roundf(color.b), (uint8_t)roundf(color.a))

void error_handler(Clay_ErrorData error) {
  printf("%s", error.errorText.chars);
  state.running = 0;
}

static inline Clay_Dimensions measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *user_data) {
  return (Clay_Dimensions){.width = text.length * CHAR_WIDTH, .height = CHAR_HEIGHT};
}

void renderer_init() {
  uint64_t total_memory = Clay_MinMemorySize();
  Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(total_memory, malloc(total_memory));
  Clay_Initialize(arena, (Clay_Dimensions){SCREEN_WIDTH, SCREEN_HEIGHT}, (Clay_ErrorHandler){error_handler});

  Clay_SetMeasureTextFunction(measure_text, NULL);
}

void execute_render_commands(Clay_RenderCommandArray render_commands) {
  for (int i = 0; i < render_commands.length; i++) {
    Clay_RenderCommand *render_command = Clay_RenderCommandArray_Get(&render_commands, i);
    Clay_BoundingBox bounding_box = render_command->boundingBox;

    switch (render_command->commandType) {
    case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
      Clay_RectangleRenderData *config = &render_command->renderData.rectangle;
      draw_rectangle(
          &(Rect){.x = bounding_box.x, .y = bounding_box.y, .w = bounding_box.width, .h = bounding_box.height},
          CLAY_COLOR_TO_PSP(config->backgroundColor));
      break;
    }

    case CLAY_RENDER_COMMAND_TYPE_TEXT: {
      Clay_TextRenderData *text_data = &render_command->renderData.text;
      draw_text_len(text_data->stringContents.chars, text_data->stringContents.length,
                    &(Vector2){.x = bounding_box.x, .y = bounding_box.y}, CLAY_COLOR_TO_PSP(text_data->textColor));
      break;
    }

    case CLAY_RENDER_COMMAND_TYPE_BORDER: {
      Clay_BorderRenderData *border_data = &render_command->renderData.border;

      if (border_data->width.left > 0)
        draw_rectangle(&(Rect){.x = bounding_box.x,
                               .y = bounding_box.y + border_data->cornerRadius.topLeft,
                               .w = border_data->width.left,
                               .h = bounding_box.height - border_data->cornerRadius.topLeft -
                                    border_data->cornerRadius.bottomLeft},
                       CLAY_COLOR_TO_PSP(border_data->color));

      if (border_data->width.right > 0)
        draw_rectangle(&(Rect){.x = bounding_box.x + bounding_box.width - border_data->width.right,
                               .y = bounding_box.y + border_data->cornerRadius.topRight,
                               .w = border_data->width.right,
                               .h = bounding_box.height - border_data->cornerRadius.topRight -
                                    border_data->cornerRadius.bottomRight},
                       CLAY_COLOR_TO_PSP(border_data->color));

      if (border_data->width.top > 0)
        draw_rectangle(
            &(Rect){.x = bounding_box.x + border_data->cornerRadius.topLeft,
                    .y = bounding_box.y,
                    .w = bounding_box.width - border_data->cornerRadius.topLeft - border_data->cornerRadius.topRight,
                    .h = border_data->width.top},
            CLAY_COLOR_TO_PSP(border_data->color));

      if (border_data->width.bottom > 0)
        draw_rectangle(&(Rect){.x = bounding_box.x + border_data->cornerRadius.bottomLeft,
                               .y = bounding_box.y + bounding_box.height - border_data->width.bottom,
                               .w = bounding_box.width - border_data->cornerRadius.bottomLeft -
                                    border_data->cornerRadius.bottomRight,
                               .h = border_data->width.bottom},
                       CLAY_COLOR_TO_PSP(border_data->color));

      break;
    }

    case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
      CustomElementData *custom_element = render_command->renderData.custom.customData;
      if (!custom_element)
        continue;

      Rect dst = {.x = bounding_box.x, .y = bounding_box.y, .w = bounding_box.width, .h = bounding_box.height};

      switch (custom_element->type) {
      case CUSTOM_ELEMENT_CARD:
        render_card(&custom_element->card, &dst);
        break;

      case CUSTOM_ELEMENT_JOKER:
        render_joker(&custom_element->joker, &dst);
        break;
      };
      break;
    }

    default:
      state.running = 0;
      break;
    }
  }
}
