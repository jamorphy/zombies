#pragma once

#include <stdbool.h>

#include "ecs.h"
#include "../libs/sokol/sokol_app.h"
#include "../libs/linmath/linmath.h"

typedef struct InputState {
    bool  keys[512];         
    float mouse_dx;         // change in mouse X this frame
    float mouse_dy;         // change in mouse Y this frame
    float mouse_x;
    float mouse_y;
    bool  mouse_captured;      // if we're capturing the mouse (typical FPS lock)
} InputState;

void input_init(InputState* input);
void input_handle_event(InputState* input, const sapp_event* ev);
void input_update(InputState* input);

void input_get_movement_direction(const InputState* input, vec3 out_dir);
void input_get_movement_vector(const InputState* input, float speed, vec3 out_vec, float yaw);

void input_process(InputState* input, Entity player, Entity camera, float delta_time);
