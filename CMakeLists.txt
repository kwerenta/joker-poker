cmake_minimum_required(VERSION 3.5)

project(joker-poker)

add_executable(${PROJECT_NAME} main.c gfx.c game.c system.c text.c content/joker.c renderer.c)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pspgu
    pspdisplay
    pspge
    pspctrl
)

# Create an EBOOT.PBP file
create_pbp_file(
    TARGET ${PROJECT_NAME}
    ICON_PATH NULL
    BACKGROUND_PATH NULL
    PREVIEW_PATH NULL
    TITLE ${PROJECT_NAME}
    VERSION 01.00
)
