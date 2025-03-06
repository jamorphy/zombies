#include "input.h"
#include "ecs.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

static float camera_yaw   = 0.0f;  // in degrees
static float camera_pitch = 0.0f;

static const float MOVE_SPEED   = 0.2f;
static const float MOUSE_SENSITIVITY = 0.2f;
static const float MAX_PITCH    = 89.0f;      // lock pitch to avoid flipping the camera

#ifndef DEG2RAD
#define DEG2RAD(x) ((x) * (3.1415926535f / 180.0f))
#endif

#define PI 3.141592653589793

void input_init(InputState* input) {
    memset(input, 0, sizeof(InputState));
    //input->mouse_locked = true;
}

void input_handle_event(InputState* state, const sapp_event* ev) {
    switch (ev->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
            if (ev->key_code < 512) {
                state->keys[ev->key_code] = true;
                if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
                    sapp_lock_mouse(false);
                    state->mouse_captured = false;
                }
            }
            break;

        case SAPP_EVENTTYPE_KEY_UP:
            if (ev->key_code < SAPP_KEYCODE_MENU + 1) {
                state->keys[ev->key_code] = false;
            }
            break;

        case SAPP_EVENTTYPE_MOUSE_DOWN:
            if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT && !state->mouse_captured) {
                sapp_lock_mouse(true);
                state->mouse_captured = true;
            }
            break;

        case SAPP_EVENTTYPE_MOUSE_MOVE:
            if (state->mouse_captured) {
                state->mouse_dx += ev->mouse_dx;
                state->mouse_dy += ev->mouse_dy;
            }
            break;

        default: break;
    }
}

/* void input_update(InputState* input) { */
/*     // input->mouse_dx = 0.0f; */
/*     // input->mouse_dy = 0.0f; */
/* } */

/*
  Movement
 */
void input_get_movement_direction(const InputState* input, vec3 out_dir) {
    out_dir[0] = 0.0f;
    out_dir[1] = 0.0f;
    out_dir[2] = 0.0f;
    
    if (input->keys[SAPP_KEYCODE_W]) out_dir[2] += 1.0f;
    if (input->keys[SAPP_KEYCODE_S]) out_dir[2] -= 1.0f;
    if (input->keys[SAPP_KEYCODE_A]) out_dir[0] -= 1.0f;
    if (input->keys[SAPP_KEYCODE_D]) out_dir[0] += 1.0f;
}

void input_get_movement_vector(const InputState* input, float speed, vec3 out_vec, float yaw) {
    vec3 dir;
    input_get_movement_direction(input, dir);
    
    // Rotate movement direction by camera yaw
    float yaw_rad = DEG2RAD(yaw);
    vec3 rotated_dir = {
        dir[0] * cosf(yaw_rad) - dir[2] * sinf(yaw_rad),
        dir[1],
        dir[0] * sinf(yaw_rad) + dir[2] * cosf(yaw_rad)
    };
    
    if (vec3_len(rotated_dir) > 0) {
        vec3_norm(rotated_dir, rotated_dir);
    }
    vec3_scale(out_vec, rotated_dir, speed);
}

void input_process(InputState* input, Entity camera, float delta_time) {
    // Constants
    const float MOUSE_SENSITIVITY = 0.1f;
    const float MOVE_SPEED = 5.0f;
    const float MAX_PITCH = 89.9f;

    CameraComponent* cam = entity_get_camera(camera);
    TransformComponent* t = entity_get_transform(camera);
    if (!cam || !t) return;

    // 1. Mouse look handling
    cam->yaw -= input->mouse_dx * MOUSE_SENSITIVITY;
    cam->pitch -= input->mouse_dy * MOUSE_SENSITIVITY;
    
    // Clamp pitch to prevent flipping
    cam->pitch = fmaxf(-MAX_PITCH, fminf(cam->pitch, MAX_PITCH));
    
    // Reset mouse deltas
    input->mouse_dx = 0.0f;
    input->mouse_dy = 0.0f;

    // 2. Camera-relative movement
    float yaw_rad = DEG2RAD(cam->yaw);
    float pitch_rad = DEG2RAD(cam->pitch);

    // Calculate orientation vectors
    vec3 forward = {
        cosf(pitch_rad) * sinf(yaw_rad),
        sinf(pitch_rad),
        cosf(pitch_rad) * cosf(yaw_rad)
    };
    
    vec3 right = {
        sinf(yaw_rad - PI/2.0f),  // Simplified right vector calculation
        0.0f,
        cosf(yaw_rad - PI/2.0f)
    };

    // Get raw input direction
    vec3 move_input;
    input_get_movement_vector(input, 1.0f, move_input, delta_time);

    // Combine vectors based on input
    vec3 world_move = {0};
    vec3 temp;
    
    // Forward/backward (W/S)
    vec3_scale(temp, forward, move_input[2]);
    vec3_add(world_move, world_move, temp);
    
    // Left/right (A/D)
    vec3_scale(temp, right, move_input[0]);
    vec3_add(world_move, world_move, temp);
    
    // Up/down (Q/E or SPACE/SHIFT if implemented)
    world_move[1] = move_input[1];

    // Apply speed and delta time
    vec3_scale(world_move, world_move, MOVE_SPEED * delta_time);
    
    // Update position
    vec3_add(t->position, t->position, world_move);
}
