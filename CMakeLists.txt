cmake_minimum_required(VERSION 3.16)

project(clayouter)

set(CMAKE_C_STANDARD 99)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG        "65c400354646fd43f54b9f832acc80ee6f63d1e2"
)

FetchContent_MakeAvailable(raylib)

add_library(clay
    STATIC
    clay/clay_renderer_raylib.c
)
target_link_libraries(clay
    raylib
)

add_executable(${PROJECT_NAME}
    src/main.c
    src/clay_enum_names.c
    src/ui_element.c
)
target_link_libraries(${PROJECT_NAME}
    clay
    raylib
    asan
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
    clay
)
target_compile_options(${PROJECT_NAME}
    PRIVATE
    -g
    -Wall
    -Wextra
    -fsanitize=address
)
