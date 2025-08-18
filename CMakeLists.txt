cmake_minimum_required(VERSION 3.11)

project(joker-poker)

add_executable(${PROJECT_NAME} main.c gfx.c game.c system.c state.c text.c renderer.c debug.c random.c utils.c content/joker.c content/tarot.c content/spectral.c)
target_include_directories(${PROJECT_NAME} PRIVATE lib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_BUILD)
endif()

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
    TITLE Joker\ Poker
    VERSION 0.30
)
