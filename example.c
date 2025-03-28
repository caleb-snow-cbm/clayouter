#include "clay/clay.h"

#define GREEN                  (Clay_Color) { 0, 255, 45, 255 }
#define DOUBLE(x)              (x * 2)
#define BACKGROUND_COLOR(a)    (Clay_Color) { a, DOUBLE(50), a - 1, 255 }

CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0),
                               .height = CLAY_SIZING_GROW(0), },
                   .padding = CLAY_PADDING_ALL(16),
                   .childGap = 16,
                   .layoutDirection = CLAY_LEFT_TO_RIGHT, },
       .backgroundColor = BACKGROUND_COLOR(DOUBLE(45)), })
{ // C++ style comment
    CLAY({ .layout = { .sizing = { .width  = CLAY_SIZING_PERCENT(0.3f),
                                   .height = CLAY_SIZING_GROW(0) },
                       .padding = CLAY_PADDING_ALL(8),
                       .childGap = 8,
                       .layoutDirection = CLAY_TOP_TO_BOTTOM },
           .backgroundColor = (Clay_Color) { 255, 255, 255, 255 },
           .cornerRadius = CLAY_CORNER_RADIUS(16),
           .border = { .color = Clay_Hovered() ? GREEN
                                               : (Clay_Color) { 0, 0, 0, 255 },
                       .width = CLAY_BORDER_OUTSIDE(2) } })
    {
        /*
         * multi-line comment
        */
        CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_FIT(0),
                                       .height = CLAY_SIZING_FIT(0) },
                           .padding = CLAY_PADDING_ALL(8), },
               .backgroundColor = Clay_Hovered() ? (Clay_Color) { 90, 220, 130, 255 }
                                                 : (Clay_Color) { 45, 190, 100, 255 },
               .cornerRadius = CLAY_CORNER_RADIUS(8), })
        {
            CLAY_TEXT(CLAY_STRING("Button 1"),
                CLAY_TEXT_CONFIG({ .fontSize = 30,
                                   .fontId = 2,
                                   .textColor = (Clay_Color) { 0, 0, 0, 255 } }));
        }
        CLAY({ .layout = { .sizing = { /* inline comment */  .width = CLAY_SIZING_GROW(0),
                                       .height = CLAY_SIZING_FIT(0) },
                           .padding = CLAY_PADDING_ALL(8), },
               .backgroundColor = (Clay_Color) { 45, 190, 100, 255 },
               .cornerRadius = CLAY_CORNER_RADIUS(8), })
        {
            CLAY_TEXT(CLAY_STRING("Button 2"),
                CLAY_TEXT_CONFIG({ .fontSize = 30,
                                   .fontId = 2,
                                   .textColor = (Clay_Color) { 0, 0, 0, 255 } }));
        }
    }
}