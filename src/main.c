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

#include "ecs.h"
#include "input.h"

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
    float q[4] = {0.0f, 0.0f, 0.0f, 1.0f };
    entity_add_transform(cube_e, (vec3){0, 0, -20}, q, (vec3){1,0.6,1});
    RenderComponent rcube = create_cube_render_component();
    entity_add_render(cube_e, rcube);

    // Player entity (cube)
    player = entity_create();
    entity_add_transform(player, (vec3){0, 0, 0}, q, (vec3){1, 1, 1});
    entity_add_render(player, rcube);

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

    entity_add_transform(camera, initial_camera_pos, q, (vec3){1, 1, 1});
    float screen_aspect = (float)sapp_width() / (float)sapp_height();
    entity_add_camera(camera, 80.0f, screen_aspect, 0.1f, 1000.0f);
    // Offset: above and behind
    entity_add_follow(camera, player, (vec3){offset_x, offset_y, offset_z});

    CameraComponent* cam = entity_get_camera(camera);
    cam->yaw = -90.0f;
    cam->pitch = 0.0f;
}

void input(const sapp_event* ev)
{
    input_handle_event(&g_input, ev);
}

void frame(void)
{
    float delta_time = sapp_frame_duration();
    input_process(&g_input, player, camera, delta_time);
    follow_system(delta_time);
    render_system(sapp_width(), sapp_height());
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
