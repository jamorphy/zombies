#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "ecs.h"
#include "HandmadeMath.h"
#include "../libs/linmath/linmath.h"

#include "cube.glsl.h"

static Registry registry = {0};
static TransformComponent transform_pool[MAX_ENTITIES];
static RenderComponent render_pool[MAX_ENTITIES];
static CameraComponent camera_pool[MAX_ENTITIES];
static FollowComponent follow_pool[MAX_ENTITIES];

#ifndef DEG2RAD
#define DEG2RAD(x) ((x) * (3.1415926535f / 180.0f))
#endif

void ecs_init()
{
    memset(&registry, 0, sizeof(Registry));
    memset(&transform_pool, 0, sizeof(transform_pool));
    memset(&render_pool, 0, sizeof(render_pool));
    memset(&camera_pool, 0, sizeof(camera_pool));
    memset(&follow_pool, 0, sizeof(follow_pool));
}

Entity entity_create()
{
    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (!registry.alive[e]) {
            registry.alive[e] = true;
            registry.component_masks[e] = 0;
            registry.entity_count++;
            return e;
        }
    }
    return INVALID_ENTITY;
}

void entity_destroy(Entity e)
{
    if (!entity_is_alive(e)) return;

    registry.alive[e] = false;
    registry.component_masks[e] = 0;
    registry.entity_count--;
}

void entity_add_transform(Entity e, vec3 pos, quat rot, vec3 scale)
{
    if (!entity_is_alive(e)) return;

    registry.component_masks[e] |= COMPONENT_TRANSFORM;

    transform_pool[e].position[0] = pos[0];
    transform_pool[e].position[1] = pos[1];
    transform_pool[e].position[2] = pos[2];

    transform_pool[e].rotation[0] = rot[0];
    transform_pool[e].rotation[1] = rot[1];
    transform_pool[e].rotation[2] = rot[2];
    transform_pool[e].rotation[3] = rot[3];

    transform_pool[e].scale[0] = scale[0];
    transform_pool[e].scale[1] = scale[1];
    transform_pool[e].scale[2] = scale[2];
}

bool entity_is_alive(Entity e) {
    // 1. Entity index within bounds
    // 2. Not marked as invalid
    // 3. Alive flag is true
    return (e < MAX_ENTITIES) && (e != INVALID_ENTITY) && registry.alive[e];
}

TransformComponent* entity_get_transform(Entity e)
{
    if (!entity_is_alive(e)) return NULL;
    if (!(registry.component_masks[e] & COMPONENT_TRANSFORM)) return NULL;
    return &transform_pool[e];
}

void entity_add_render(Entity e, RenderComponent component) {
    if (!entity_is_alive(e)) return;

    registry.component_masks[e] |= COMPONENT_RENDER;
    render_pool[e] = component;
}

RenderComponent create_cube_render_component(void) {
    static float vertices[] = {
        // positions      // colors (RGBA)
        // front
        -1, -1,  1,   1,0,0,1,
         1, -1,  1,   0,1,0,1,
         1,  1,  1,   0,0,1,1,
        -1,  1,  1,   1,1,0,1,
        // back
        -1, -1, -1,   1,0,1,1,
         1, -1, -1,   0,1,1,1,
         1,  1, -1,   0.5f,0.5f,0.5f,1,
        -1,  1, -1,   0,0,0,1
    };
    static uint16_t indices[] = {
        // front
        0,1,2,  0,2,3,
        // right
        1,5,6,  1,6,2,
        // back
        5,4,7,  5,7,6,
        // left
        4,0,3,  4,3,7,
        // bottom
        4,5,1,  4,1,0,
        // top
        3,2,6,  3,6,7
    };

    RenderComponent rc = {0};

    rc.vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data  = SG_RANGE(vertices),
        .label = "cube-vertices"
    });
    rc.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type  = SG_BUFFERTYPE_INDEXBUFFER,
        .data  = SG_RANGE(indices),
        .label = "cube-indices"
    });
    rc.index_count = sizeof(indices)/sizeof(indices[0]);

    sg_shader shd = sg_make_shader(cube_shader_desc(sg_query_backend()));

    rc.pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .layout = {
            .attrs[0] = { .format = SG_VERTEXFORMAT_FLOAT3 },
            .attrs[1] = { .format = SG_VERTEXFORMAT_FLOAT4 },
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode  = SG_CULLMODE_FRONT,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
        .label = "cube-pipeline"
    });

    return rc;
}

void entity_add_camera(Entity e, float fov, float aspect, float near_plane, float far_plane)
{
    if (!entity_is_alive(e)) return;
    registry.component_masks[e] |= COMPONENT_CAMERA;
    camera_pool[e].fov         = fov;
    camera_pool[e].aspect      = aspect;
    camera_pool[e].near_plane  = near_plane;
    camera_pool[e].far_plane   = far_plane;
}

CameraComponent* entity_get_camera(Entity e)
{
    if (!entity_is_alive(e)) return NULL;
    if (!(registry.component_masks[e] & COMPONENT_CAMERA)) return NULL;
    return &camera_pool[e];
}

void entity_add_follow(Entity e, Entity target, vec3 offset) {
    if (!entity_is_alive(e)) return;
    registry.component_masks[e] |= COMPONENT_FOLLOW;
    follow_pool[e].target = target;
    follow_pool[e].offset[0] = offset[0];
    follow_pool[e].offset[1] = offset[1];
    follow_pool[e].offset[2] = offset[2];
}

void follow_system(float delta_time)
{
    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (!entity_is_alive(e)) continue;
        if (registry.component_masks[e] & COMPONENT_FOLLOW) {
            FollowComponent* follow = &follow_pool[e];
            TransformComponent* cam_t = entity_get_transform(e);
            CameraComponent* cam = entity_get_camera(e);
            TransformComponent* target_t = entity_get_transform(follow->target);

            if (!cam || !cam_t || !target_t) continue;

            float yaw_rad = DEG2RAD(cam->yaw);
            float pitch_rad = DEG2RAD(cam->pitch);

            vec3 forward = {
                cosf(pitch_rad) * sinf(yaw_rad),
                sinf(pitch_rad),
                cosf(pitch_rad) * cosf(yaw_rad)
            };

            float distance = 5.0f;
            vec3 offset;
            vec3_scale(offset, forward, -distance);
            vec3_add(cam_t->position, target_t->position, offset);
        }
    }
}

void render_system(int width, int height)
{
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

    mat4x4 view, proj;
    bool camera_found = false;

    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (!entity_is_alive(e)) {
            continue;
        }
        if (registry.component_masks[e] & COMPONENT_CAMERA) {
            CameraComponent* cam = &camera_pool[e];
            TransformComponent* ct = &transform_pool[e];

            FollowComponent* follow = &follow_pool[e];
            TransformComponent* target_t = entity_get_transform(follow->target);

            if (!ct || !cam || !target_t) continue;

            vec3 position = {ct->position[0], ct->position[1], ct->position[2]};
            vec3 target = {target_t->position[0], target_t->position[1], target_t->position[2]};
            vec3 up = {0.0f, 1.0f, 0.0f};

            mat4x4_look_at(view, position, target, up);

            float fovy_rad = cam->fov * (3.1415926535f / 180.0f);
            mat4x4_perspective(proj, fovy_rad, cam->aspect, cam->near_plane, cam->far_plane);

            camera_found = true;
            break;
        }
    }

    if (!camera_found) {
        vec3 eye    = { 0.0f, 2.0f, 5.0f };
        vec3 center = { 0.0f, 0.0f, 0.0f };
        vec3 up     = { 0.0f, 1.0f, 0.0f };

        mat4x4_look_at(view, eye, center, up);

        float fovy_rad = 45.0f * (3.1415926535f / 180.0f);
        float aspect   = (float) width / (float) height;
        mat4x4_perspective(proj, fovy_rad, aspect, 0.1f, 100.0f);
    }

    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (!entity_is_alive(e)) {
            continue;
        }
        uint32_t mask = registry.component_masks[e];
        if ((mask & (COMPONENT_TRANSFORM | COMPONENT_RENDER)) ==
                        (COMPONENT_TRANSFORM | COMPONENT_RENDER))
        {
            TransformComponent* t = &transform_pool[e];
            RenderComponent*    r = &render_pool[e];

            mat4x4 model;
            mat4x4_identity(model);

            mat4x4_translate_in_place(model,
                t->position[0],
                t->position[1],
                t->position[2]);

            mat4x4 rot;
            mat4x4_from_quat(rot, t->rotation);
            mat4x4_mul(model, model, rot);

            mat4x4 scale;
            mat4x4_identity(scale);
            scale[0][0] = t->scale[0];
            scale[1][1] = t->scale[1];
            scale[2][2] = t->scale[2];
            mat4x4_mul(model, model, scale);

            mat4x4 mv, mvp;
            mat4x4_mul(mv, view, model);
            mat4x4_mul(mvp, proj, mv);

            sg_apply_pipeline(r->pipeline);
            sg_apply_uniforms(0, &SG_RANGE(mvp));
            sg_bindings bind = {
                .vertex_buffers[0] = r->vertex_buffer,
                .index_buffer      = r->index_buffer
            };
            sg_apply_bindings(&bind);
            sg_draw(0, r->index_count, 1);
        }
    }

    sg_end_pass();
    sg_commit();
}
