#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include "clay.h"
#include "clay_renderer_raylib.h"

#define WINDOW_WIDTH    (1024)
#define WINDOW_HEIGHT   (768)

typedef struct ui_element_s {
    Clay_ElementDeclaration* ptr;
    struct ui_element_s** children;
    size_t num_children;
} ui_element_t;

void clay_error(Clay_ErrorData err)
{
    fprintf(stderr, "CLAY ERROR: %d, %.*s\n",
        (int) err.errorType, (int) err.errorText.length, err.errorText.chars);
    exit(1);
}

void hover_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    (void) user_data;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        
    }
}

void configure_element(ui_element_t* me)
{
    if (me == NULL) {
        return;
    }
    Clay__OpenElement();
    Clay__ConfigureOpenElement(*me->ptr);
    for (size_t i = 0; i < me->num_children; ++i) {
        Clay_OnHover(hover_callback, NULL);
        configure_element(me->children[i]);
    }
    Clay__CloseElement();
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Clayouter");
    uint64_t clay_memory_size = Clay_MinMemorySize();
    void* clay_memory = malloc(clay_memory_size);
    Clay_Arena clay_arena = Clay_CreateArenaWithCapacityAndMemory(clay_memory_size, clay_memory);

    Clay_ErrorHandler err = {
        .errorHandlerFunction = clay_error,
        .userData = NULL
    };
    Clay_Initialize(clay_arena, (Clay_Dimensions) { WINDOW_WIDTH, WINDOW_HEIGHT }, err);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, NULL);

    ui_element_t root = { 0 };
    root.ptr = (Clay_ElementDeclaration*) malloc(sizeof(Clay_ElementDeclaration));
    *root.ptr = (Clay_ElementDeclaration) {
        .id = CLAY_ID("root")
    };

    while (!WindowShouldClose()) {
        Clay_SetLayoutDimensions((Clay_Dimensions) { GetScreenWidth(), GetScreenHeight() });
        Clay_SetPointerState(RAYLIB_VECTOR_TO_CLAY_VECTOR(GetMousePosition()), IsMouseButtonDown(0));

        Clay_BeginLayout();
        configure_element(&root);

        BeginDrawing();
        ClearBackground(BLACK);
        Clay_Raylib_Render(Clay_EndLayout(), NULL);
        EndDrawing();
    }
    CloseWindow();
    free(clay_memory);
}
