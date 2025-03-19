cmake_minimum_required(VERSION 3.16)

project(clayouter)

set(CMAKE_C_STANDARD 99)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_SHALLOW     1
)

FetchContent_MakeAvailable(raylib)

add_library(clay
    STATIC
    clay/clay_renderer_raylib.c
)
target_include_directories(clay
    PRIVATE
    ${raylib_SOURCE_DIR}/src
)

add_executable(${PROJECT_NAME}
    src/main.c
    src/clay_enum_names.c
    src/clay_struct_names.c
    src/ui_element.c
    src/components/clay_components.c
    src/IO/import_layout.c
    src/IO/export_layout.c
)
target_link_libraries(${PROJECT_NAME}
    clay
    raylib
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    clay
    src
    lib
)
