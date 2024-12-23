cmake_minimum_required(VERSION 3.5)

project(joker-poker)

find_package(SDL2 REQUIRED SDL2)
include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS})

# target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 m)

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
