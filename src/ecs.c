#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "ecs.h"
#include "HandmadeMath.h"
#include "macros.h"
#include "../libs/linmath/linmath.h"

#include "cube.glsl.h"

#include "transform.h"
#include "render.h"
#include "camera.h"
#include "physics.h"
#include "projectile.h"

Registry registry = {0};
static TransformComponent transform_pool[MAX_ENTITIES];
static RenderComponent render_pool[MAX_ENTITIES];
static CameraComponent camera_pool[MAX_ENTITIES];
static FollowComponent follow_pool[MAX_ENTITIES];
static CollisionComponent collision_pool[MAX_ENTITIES];
static ProjectileComponent projectile_pool[MAX_ENTITIES];
static VelocityComponent velocity_pool[MAX_ENTITIES];
static LifetimeComponent lifetime_pool[MAX_ENTITIES];
static HealthComponent health_pool[MAX_ENTITIES];
static DamageComponent damage_pool[MAX_ENTITIES];

// TODO: Move health and damage components to other game logic stuff
ECS_COMPONENT_ACCESSORS(health, HealthComponent, COMPONENT_HEALTH)
ECS_COMPONENT_ACCESSORS(damage, DamageComponent, COMPONENT_DAMAGE)

void ecs_init()
{
    memset(&registry, 0, sizeof(Registry));
    memset(&transform_pool, 0, sizeof(transform_pool));
    memset(&render_pool, 0, sizeof(render_pool));
    memset(&camera_pool, 0, sizeof(camera_pool));
    memset(&follow_pool, 0, sizeof(follow_pool));
    memset(&collision_pool, 0, sizeof(collision_pool));
    memset(&projectile_pool, 0, sizeof(projectile_pool));
    memset(&velocity_pool, 0, sizeof(velocity_pool));
    memset(&lifetime_pool, 0, sizeof(lifetime_pool));
    memset(&health_pool, 0, sizeof(health_pool));
    memset(&damage_pool, 0, sizeof(damage_pool));
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

    // (dirty): Don't destroy render components everytime an entity dies
    // A couple options:
    // Ref counts, only destroy when nothing is ref a RenderComponent
    // every entity has it's own RenderComponent
    /* if (registry.component_masks[e] & COMPONENT_RENDER) { */
    /*     RenderComponent* rc = &render_pool[e]; */
    /*     sg_destroy_buffer(rc->vertex_buffer); */
    /*     sg_destroy_buffer(rc->index_buffer); */
    /*     // Note: Do NOT destroy rc->pipeline here, as it's now shared */
    /* } */

    registry.alive[e] = false;
    registry.component_masks[e] = 0;
    registry.entity_count--;
}

bool entity_is_alive(Entity e)
{
    // 1. Entity index within bounds
    // 2. Not marked as invalid
    // 3. Alive flag is true
    return (e < MAX_ENTITIES) && (e != INVALID_ENTITY) && registry.alive[e];
}

void* ecs_get_component(Entity e, ComponentType type)
{
    if (!entity_is_alive(e) || !(registry.component_masks[e] & type)) return NULL;
    switch (type) {
        case COMPONENT_TRANSFORM: return &transform_pool[e];
        case COMPONENT_RENDER: return &render_pool[e];
        case COMPONENT_CAMERA: return &camera_pool[e];
        case COMPONENT_FOLLOW: return &follow_pool[e];
        case COMPONENT_COLLISION: return &collision_pool[e];
        case COMPONENT_PROJECTILE: return &projectile_pool[e];
        case COMPONENT_VELOCITY: return &velocity_pool[e];
        case COMPONENT_LIFETIME: return &lifetime_pool[e];
        case COMPONENT_HEALTH: return &health_pool[e];
        case COMPONENT_DAMAGE: return &damage_pool[e];
        // Other cases...
        default: return NULL;
    }
}

void ecs_set_component(Entity e, ComponentType type, void* component)
{
    if (!entity_is_alive(e)) return;
    registry.component_masks[e] |= type;
    switch (type) {
        case COMPONENT_TRANSFORM:
            transform_pool[e] = *(TransformComponent*)component;
            break;
        case COMPONENT_RENDER:
            render_pool[e] = *(RenderComponent*)component;
            break;
        case COMPONENT_CAMERA:
            camera_pool[e] = *(CameraComponent*)component;
            break;
        case COMPONENT_FOLLOW:
            follow_pool[e] = *(FollowComponent*)component;
            break;
        case COMPONENT_COLLISION:
            collision_pool[e] = *(CollisionComponent*)component;
            break;
        case COMPONENT_PROJECTILE:
            projectile_pool[e] = *(ProjectileComponent*)component;
            break;
        case COMPONENT_VELOCITY:
            velocity_pool[e] = *(VelocityComponent*)component;
            break;
        case COMPONENT_LIFETIME:
            lifetime_pool[e] = *(LifetimeComponent*)component;
            break;
        case COMPONENT_HEALTH:
            health_pool[e] = *(HealthComponent*)component;
            break;
        case COMPONENT_DAMAGE:
            damage_pool[e] = *(DamageComponent*)component;
            break;
        // Other components...
    }
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

            float fovy_rad = cam->fov * (PI / 180.0f);
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
}

