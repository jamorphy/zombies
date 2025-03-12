#include <stdio.h>
#include "gui.h"
#include "transform.h"

#define NUKLEAR_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_COMMAND_USERDATA
#include "../libs/nuklear/nuklear.h"
#include "../libs/sokol/sokol_app.h"
#include "../libs/sokol/sokol_nuklear.h"

void gui_render(Entity player)
{
    struct nk_context* ctx = snk_new_frame();
    
    if (nk_begin(ctx, "Player Transform", nk_rect(25, 25, 225, 200),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 20, 1);

        TransformComponent* t = entity_get_transform(player);
        if (t) {
            char pos_str[64];
            snprintf(pos_str, sizeof(pos_str), "Pos: %.2f, %.2f, %.2f",
                     t->position[0], t->position[1], t->position[2]);
            nk_label(ctx, pos_str, NK_TEXT_LEFT);

            char rot_str[64];
            snprintf(rot_str, sizeof(rot_str), "Rot: %.2f, %.2f, %.2f, %.2f",
                     t->rotation[0], t->rotation[1], t->rotation[2], t->rotation[3]);
            nk_label(ctx, rot_str, NK_TEXT_LEFT);

            char scale_str[64];
            snprintf(scale_str, sizeof(scale_str), "Scale: %.2f, %.2f, %.2f",
                     t->scale[0], t->scale[1], t->scale[2]);
            nk_label(ctx, scale_str, NK_TEXT_LEFT);
        } else {
            nk_label(ctx, "No transform data", NK_TEXT_LEFT);
        }
    }
    nk_end(ctx);
}
