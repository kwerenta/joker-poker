cmake_minimum_required(VERSION 3.5)

project(joker-poker)

find_package(SDL2 REQUIRED SDL2)
find_package(SDL2_IMAGE REQUIRED SDL2_image)

add_executable(${PROJECT_NAME} main.c gfx.c window.c game.c)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2_image::SDL2_image)

if(PSP)
    create_pbp_file(
        TARGET ${PROJECT_NAME}
        ICON_PATH NULL
        BACKGROUND_PATH NULL
        PREVIEW_PATH NULL
        TITLE ${PROJECT_NAME}
        VERSION 01.00
    )
endif()
