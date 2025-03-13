#include "projectile.h"
#include "transform.h"
#include "camera.h"
#include "physics.h"
#include "render.h"
#include "macros.h"

ECS_COMPONENT_ACCESSORS(projectile, ProjectileComponent, COMPONENT_PROJECTILE)

void create_projectile(Entity shooter, Entity camera_entity)
{
    Entity projectile = entity_create();
    if (projectile == INVALID_ENTITY) return;

    TransformComponent* player_t = entity_get_transform(shooter);
    CameraComponent* cam = entity_get_camera(camera_entity);

    // Calculate spawn position: offset from player center along camera forward direction
    float yaw_rad = DEG2RAD(cam->yaw);
    float pitch_rad = DEG2RAD(cam->pitch);
    vec3 forward = {
        cosf(pitch_rad) * sinf(yaw_rad),
        sinf(pitch_rad),
        cosf(pitch_rad) * cosf(yaw_rad)
    };
    vec3_norm(forward, forward); // Ensure unit vector

    // Offset by player size (assuming scale {1, 1, 1} and collision size {1, 1, 1}) plus buffer
    float offset_distance = 1.5f; // Player half-size (0.5) + projectile half-size (0.1) + buffer
    vec3 offset;
    vec3_scale(offset, forward, offset_distance);
    vec3 projectile_pos;
    vec3_add(projectile_pos, player_t->position, offset);

    vec3 scale = {0.2f, 0.2f, 0.2f};
    float rot[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    TransformComponent t = {
        .position = {projectile_pos[0], projectile_pos[1], projectile_pos[2]},
        .rotation = {rot[0], rot[1], rot[2], rot[3]},
        .scale = {scale[0], scale[1], scale[2]}
    };
    entity_set_transform(projectile, t);

    // Velocity: Same forward direction, just faster
    float speed = 20.0f;
    vec3 velocity;
    vec3_scale(velocity, forward, speed);
    VelocityComponent v = { .velocity = {velocity[0], velocity[1], velocity[2]} };
    entity_set_velocity(projectile, v);

    RenderComponent rcube = create_cube_render_component();
    entity_set_render(projectile, rcube);

    CollisionComponent c = { .size = {1.0f, 1.0f, 1.0f}, .center_offset = {0, 0, 0}, .is_static = false };
    entity_set_collision(projectile, c);

    ProjectileComponent p = { .owner = shooter };
    entity_set_projectile(projectile, p);

    LifetimeComponent l = { .lifetime = 5.0f };
    entity_set_lifetime(projectile, l);
}
