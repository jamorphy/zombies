#pragma once

#include "ecs.h"
#include "transform.h"
#include "../libs/linmath/linmath.h"

typedef struct {
    vec3 min;           // AABB min bounds (world space)
    vec3 max;           // AABB max bounds (world space)
    vec3 size;          // Local-space full size (before scaling)
    vec3 center_offset; // Offset from transform position
    bool is_static;     // True for immovable objects (e.g., ground, walls)
} CollisionComponent;

typedef struct {
    vec3 velocity;
} VelocityComponent;

typedef struct {
    float lifetime;
} LifetimeComponent; 

void physics_init(void);

CollisionComponent* entity_get_collision(Entity e);
void entity_set_collision(Entity e, CollisionComponent component);

VelocityComponent* entity_get_velocity(Entity e);
void entity_set_velocity(Entity e, VelocityComponent component);

LifetimeComponent* entity_get_lifetime(Entity e);
void entity_set_lifetime(Entity e, LifetimeComponent component);

void physics_update_collision_transform(TransformComponent* transform, CollisionComponent* collision);
bool physics_check_aabb_collision(const vec3 min1, const vec3 max1, const vec3 min2, const vec3 max2);
void physics_system_update(float delta_time);
