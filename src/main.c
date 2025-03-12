#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "linmath.h"

#define SOKOL_METAL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "../libs/nuklear/nuklear.h"
#include "../libs/sokol/sokol_nuklear.h"

#include "ecs.h"
#include "transform.h"
#include "input.h"
#include "gui.h"
#include "render.h"

static InputState g_input;
static Entity player;
static Entity camera;
static Entity cube;

void cleanup(void);

void init(void)
{
    sg_setup(&(sg_desc){ .environment = sglue_environment(), .logger.func = slog_func });
    ecs_init();
    input_init(&g_input);

    cube = entity_create();

    Entity cube_e = entity_create();
    assert(entity_is_alive(cube_e));
    float rot[4] = {0.0f, 0.0f, 0.0f, 1.0f };
    vec3 scale = {1, 1, 1};
    vec3 pos1 = { 0, 0, 20 };
    vec3 pos2 = { 0, 0, 0 };
    TransformComponent t = { .position = {pos1[0], pos1[1], pos1[2]}, .rotation = {rot[0], rot[1], rot[2], rot[3]}, .scale = {scale[0], scale[1], scale[2]} };
    entity_set_transform(cube_e, t);
    RenderComponent rcube = create_cube_render_component();
    entity_set_render(cube_e, rcube);

    // Player entity (cube)
    player = entity_create();
    TransformComponent t1 = { .position = {pos2[0], pos2[1], pos2[2]}, .rotation = {rot[0], rot[1], rot[2], rot[3]}, .scale = {scale[0], scale[1], scale[2]} };
    entity_set_transform(player, t1);
    entity_set_render(player, rcube);

    camera = entity_create();

    float offset_x = 0.0f;
    float offset_y = 5.0f;
    float offset_z = -5.0f;

    TransformComponent* player_t = entity_get_transform(player);
    vec3 initial_camera_pos = {
        player_t->position[0] + offset_x,
        player_t->position[1] + offset_y,
        player_t->position[2] + offset_z,
    };

    TransformComponent t2 = { .position = {initial_camera_pos[0], initial_camera_pos[1], initial_camera_pos[2]}, .rotation = {rot[0], rot[1], rot[2], rot[3]}, .scale = {scale[0], scale[1], scale[2]} };
    entity_set_transform(camera, t2);
    float screen_aspect = (float)sapp_width() / (float)sapp_height();
    entity_add_camera(camera, 80.0f, screen_aspect, 0.1f, 1000.0f);
    // Offset: above and behind
    entity_add_follow(camera, player, (vec3){offset_x, offset_y, offset_z});

    CameraComponent* cam = entity_get_camera(camera);
    cam->yaw = -90.0f;
    cam->pitch = 0.0f;

    snk_setup(&(snk_desc_t){0});
    nk_style_hide_cursor(snk_new_frame());
}

void input(const sapp_event* ev)
{
    snk_handle_event(ev);
    input_handle_event(&g_input, ev);
}

void frame(void)
{
    float delta_time = sapp_frame_duration();
    input_process(&g_input, player, camera, delta_time);
    follow_system(delta_time);

    sg_begin_pass(&(sg_pass){
        .action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.25f, 0.5f, 0.75f, 1.0f }
            },
            .depth = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = 1.0f
            }
        },
        .swapchain = sglue_swapchain()
    });
    
    render_system(sapp_width(), sapp_height());
    gui_render(player);

    snk_render(sapp_width(),sapp_height());
    sg_end_pass();
    sg_commit();
}

void cleanup(void)
{
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[])
{
    int WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 960;
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = input,
        .width = WINDOW_WIDTH,
        .height = WINDOW_HEIGHT,
        .sample_count = 4,
        .window_title = "eg",
        .icon.sokol_default = false,
        .swap_interval = 1
    };
}
