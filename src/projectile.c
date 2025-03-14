#include "projectile.h"
#include "transform.h"
#include "camera.h"
#include "physics.h"
#include "render.h"
#include "macros.h"
#include "event.h"

ECS_COMPONENT_ACCESSORS(projectile, ProjectileComponent, COMPONENT_PROJECTILE)

static void on_shoot(void* data)
{
    ShootEvent* ev = (ShootEvent*)data;
    create_projectile(ev->shooter, ev->position, ev->direction);
}

void projectile_init(void)
{
    event_register(EVENT_SHOOT, on_shoot); // pass func ptr for on_shoot
}

void create_projectile(Entity shooter, vec3 position, vec3 direction) {
    Entity projectile = entity_create();
    if (projectile == INVALID_ENTITY) return;

    // Offset projectile from shooter position along direction
    vec3 offset;
    vec3_scale(offset, direction, 1.5f); // 1.5f = shooter half-size + buffer
    vec3 projectile_pos;
    vec3_add(projectile_pos, position, offset);

    vec3 scale = {0.2f, 0.2f, 0.2f};
    float rot[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    TransformComponent t = {
        .position = {projectile_pos[0], projectile_pos[1], projectile_pos[2]},
        .rotation = {rot[0], rot[1], rot[2], rot[3]},
        .scale = {scale[0], scale[1], scale[2]}
    };
    entity_set_transform(projectile, t);

    float speed = 20.0f;
    vec3 velocity;
    vec3_scale(velocity, direction, speed);
    VelocityComponent v = { .velocity = {velocity[0], velocity[1], velocity[2]} };
    entity_set_velocity(projectile, v);

    static float vertices[] = {
        -1, -1,  1,   1,0,0,1,
         1, -1,  1,   0,1,0,1,
         1,  1,  1,   0,0,1,1,
        -1,  1,  1,   1,1,0,1,
        -1, -1, -1,   1,0,1,1,
         1, -1, -1,   0,1,1,1,
         1,  1, -1,   0.5f,0.5f,0.5f,1,
        -1,  1, -1,   0,0,0,1
    };
    static uint16_t indices[] = {
        0,1,2,  0,2,3,
        1,5,6,  1,6,2,
        5,4,7,  5,7,6,
        4,0,3,  4,3,7,
        4,5,1,  4,1,0,
        3,2,6,  3,6,7
    };
    RenderComponent rc = create_render_component(
        vertices, sizeof(vertices),
        indices, sizeof(indices) / sizeof(indices[0])
    );
    entity_set_render(projectile, rc);

    CollisionComponent c = {
        .size = {1.0f, 1.0f, 1.0f},
        .center_offset = {0, 0, 0},
        .is_static = false
    };
    entity_set_collision(projectile, c);

    ProjectileComponent p = { .owner = shooter };
    entity_set_projectile(projectile, p);

    LifetimeComponent l = { .lifetime = 5.0f };
    entity_set_lifetime(projectile, l);
}
