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
#include "camera.h"
#include "physics.h"

static InputState g_input;
static Entity player;
static Entity camera;
static Entity cube;

static int frame_count;

void cleanup(void);

void init(void)
{
    sg_setup(&(sg_desc){ .environment = sglue_environment(), .logger.func = slog_func });
    ecs_init();
    input_init(&g_input);

    cube = entity_create();

    // Original cube
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
    entity_set_collision(cube_e, (CollisionComponent){.size = {1, 1, 1}, .center_offset = {0, 0, 0}, .is_static = true});

    // New cube 1: Tall thin pillar
    Entity cube1 = entity_create();
    vec3 scale1 = {0.5f, 3.0f, 0.5f};  // Thin and tall
    vec3 pos_c1 = {50.0f, 0.0f, 30.0f};  // Distance: ~58 units
    TransformComponent t_c1 = { .position = {pos_c1[0], pos_c1[1], pos_c1[2]}, .rotation = {rot[0], rot[1], rot[2], rot[3]}, .scale = {scale1[0], scale1[1], scale1[2]} };
    entity_set_transform(cube1, t_c1);
    entity_set_render(cube1, rcube);
    entity_set_collision(cube1, (CollisionComponent){.size = {1, 1, 1}, .center_offset = {0, 0, 0}, .is_static = true});

    // New cube 2: Wide flat platform
    Entity cube2 = entity_create();
    vec3 scale2 = {4.0f, 0.2f, 4.0f};  // Wide and flat
    vec3 pos_c2 = {-30.0f, -1.0f, -40.0f};  // Distance: ~50 units
    TransformComponent t_c2 = { .position = {pos_c2[0], pos_c2[1], pos_c2[2]}, .rotation = {rot[0], rot[1], rot[2], rot[3]}, .scale = {scale2[0], scale2[1], scale2[2]} };
    entity_set_transform(cube2, t_c2);
    entity_set_render(cube2, rcube);
    entity_set_collision(cube2, (CollisionComponent){.size = {1, 1, 1}, .center_offset = {0, 0, 0}, .is_static = true});

    // New cube 3: Long wall
    Entity cube3 = entity_create();
    vec3 scale3 = {6.0f, 2.0f, 0.5f};  // Long and thin
    vec3 pos_c3 = {20.0f, 0.0f, -60.0f};  // Distance: ~63 units
    TransformComponent t_c3 = { .position = {pos_c3[0], pos_c3[1], pos_c3[2]}, .rotation = {rot[0], rot[1], rot[2], rot[3]}, .scale = {scale3[0], scale3[1], scale3[2]} };
    entity_set_transform(cube3, t_c3);
    entity_set_render(cube3, rcube);
    entity_set_collision(cube3, (CollisionComponent){.size = {1, 1, 1}, .center_offset = {0, 0, 0}, .is_static = true});

    // New cube 4: Medium box
    Entity cube4 = entity_create();
    vec3 scale4 = {2.0f, 2.0f, 2.0f};  // Larger cube
    vec3 pos_c4 = {-80.0f, 0.0f, 20.0f};  // Distance: ~82 units
    TransformComponent t_c4 = { .position = {pos_c4[0], pos_c4[1], pos_c4[2]}, .rotation = {rot[0], rot[1], rot[2], rot[3]}, .scale = {scale4[0], scale4[1], scale4[2]} };
    entity_set_transform(cube4, t_c4);
    entity_set_render(cube4, rcube);
    entity_set_collision(cube4, (CollisionComponent){.size = {1, 1, 1}, .center_offset = {0, 0, 0}, .is_static = true});

    // New cube 5: Small floating block
    Entity cube5 = entity_create();
    vec3 scale5 = {0.7f, 0.7f, 0.7f};  // Small cube
    vec3 pos_c5 = {10.0f, 5.0f, 50.0f};  // Distance: ~51 units
    TransformComponent t_c5 = { .position = {pos_c5[0], pos_c5[1], pos_c5[2]}, .rotation = {rot[0], rot[1], rot[2], rot[3]}, .scale = {scale5[0], scale5[1], scale5[2]} };
    entity_set_transform(cube5, t_c5);
    entity_set_render(cube5, rcube);
    entity_set_collision(cube5, (CollisionComponent){.size = {1, 1, 1}, .center_offset = {0, 0, 0}, .is_static = true});

    // Player entity (cube)
    player = entity_create();
    TransformComponent t1 = { .position = {pos2[0], pos2[1], pos2[2]}, .rotation = {rot[0], rot[1], rot[2], rot[3]}, .scale = {scale[0], scale[1], scale[2]} };
    entity_set_transform(player, t1);
    entity_set_render(player, rcube);
    entity_set_collision(player, (CollisionComponent){.size = {1, 1, 1}, .center_offset = {0, 0, 0}, .is_static = false});

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
    CameraComponent cam = { .fov = 80.0f, .aspect = screen_aspect, .near_plane = 0.1f, .far_plane = 1000.0f, .pitch = 0.0f, .yaw = -90.0f };
    entity_set_camera(camera, cam);

    // Offset: above and behind
    FollowComponent fol = { .target = player, .offset = {offset_x, offset_y, offset_z }};
    entity_set_follow(camera, fol);

    //CameraComponent* cam = entity_get_camera(camera);

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
    frame_count++;
    float delta_time = sapp_frame_duration();
    input_process(&g_input, player, camera, delta_time);
    physics_system_update(delta_time);
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
