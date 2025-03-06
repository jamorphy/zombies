#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "linmath.h"
#include "cJSON.h"

#define SOKOL_METAL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "sokol_debugtext.h"

#include "log.h"
#include "ecs.h"
#include "input.h"

static InputState g_input;
static Entity camera;

void cleanup(void);

void init(void)
{
    sg_setup(&(sg_desc){ .environment = sglue_environment(), .logger.func = slog_func });

    input_init(&g_input);

    camera = entity_create();
    float q1[4] = {0.0f, 0.0f, 0.0f, 1.0f };
    entity_add_transform(camera, (vec3){0, 0, 0}, q1, (vec3){0,2,5});
    float screen_aspect = (float) sapp_width() / (float) sapp_height();
    entity_add_camera(
                      camera,
                      60.0f,        // FOV in degrees
                      screen_aspect,
                      0.1f,         // near clipping plane
                      1000.0f       // far clipping plane
                      );

    CameraComponent* cam = entity_get_camera(camera);
    cam->yaw = 180.0f; // Look along -Z axis
    cam->pitch = 0.0f;    

    Entity cube_e = entity_create();
    assert(entity_is_alive(cube_e));
    float q[4] = {0.0f, 0.0f, 0.0f, 1.0f };
    entity_add_transform(cube_e, (vec3){0, 0, -20}, q, (vec3){1,0.6,1});
    RenderComponent rcube = create_cube_render_component();
    entity_add_render(cube_e, rcube);

    Entity cube_e1 = entity_create();
    assert(entity_is_alive(cube_e1));
    entity_add_transform(cube_e1, (vec3){0, 20, -20}, q, (vec3){1,0.6,1});
    entity_add_render(cube_e1, rcube);
}

void input(const sapp_event* ev)
{
    input_handle_event(&g_input, ev);
}

void frame(void)
{
    float delta_time = sapp_frame_duration();
    input_process(&g_input, camera, delta_time);
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
