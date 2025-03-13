#include "input.h"
#include "ecs.h"
#include "transform.h"
#include "camera.h"
#include "macros.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

static const float MOVE_SPEED   = 6.0f;
static const float MOUSE_SENSITIVITY = 0.2f;
static const float MAX_PITCH    = 89.0f;      // lock pitch to avoid flipping the camera

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

/*
  Movement
 */
void input_get_movement_direction(const InputState* input, vec3 out_dir) {
    out_dir[0] = 0.0f;
    out_dir[1] = 0.0f;
    out_dir[2] = 0.0f;

    if (input->keys[SAPP_KEYCODE_W]) out_dir[2] += 1.0f;
    if (input->keys[SAPP_KEYCODE_S]) out_dir[2] -= 1.0f;
    if (input->keys[SAPP_KEYCODE_A]) out_dir[0] += 1.0f;
    if (input->keys[SAPP_KEYCODE_D]) out_dir[0] -= 1.0f;
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

void input_process(InputState* input, Entity player, Entity camera, float delta_time)
{
    TransformComponent* t = entity_get_transform(player);
    CameraComponent* cam = entity_get_camera(camera);
    if (!t) return;

    static float player_yaw = 0.0f;
    static float camera_pitch = 0.0f;

    // Left = +yaw (CCW), Right = -yaw (CW)
    player_yaw -= input->mouse_dx * MOUSE_SENSITIVITY;
    if (player_yaw >= 360.0f) player_yaw -= 360.0f;
    if (player_yaw < 0.0f) player_yaw += 360.0f;

    camera_pitch -= input->mouse_dy * MOUSE_SENSITIVITY;
    if (camera_pitch > MAX_PITCH) camera_pitch = MAX_PITCH;
    if (camera_pitch < -MAX_PITCH) camera_pitch = -MAX_PITCH;

    cam->yaw = player_yaw;
    cam->pitch = camera_pitch;

    // Create player rotation quaternion
    quat_rotate(t->rotation, DEG2RAD(player_yaw), (vec3){0, 1, 0}); // tilt around Y(aw)

    // Local-space direction e.g., set base_dir W = (0, 0, 1)
    vec3 base_dir = {0.0f, 0.0f, 0.0f};
    input_get_movement_direction(input, base_dir);

    // Rotate base direction by player's quaternion
    // get world-space direction
    vec3 move_dir;
    quat_mul_vec3(move_dir, t->rotation, base_dir);

    // Scale by speed and delta time
    vec3 move_input;
    vec3_scale(move_input, move_dir, MOVE_SPEED * delta_time);

    vec3_add(t->position, t->position, move_input);

    input->mouse_dx = 0.0;
    input->mouse_dy = 0.0;
}
