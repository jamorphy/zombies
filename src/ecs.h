#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "linmath.h"

#include "../libs/sokol/sokol_gfx.h"
#include "../libs/sokol/sokol_glue.h"
#include "../libs/linmath/linmath.h"

#define MAX_ENTITIES 4096

typedef uint32_t Entity;
#define INVALID_ENTITY MAX_ENTITIES

typedef enum ComponentType {
    COMPONENT_NONE = 0,
    COMPONENT_TRANSFORM = 1 << 0,
    COMPONENT_RENDER = 1 << 1,
    COMPONENT_CAMERA = 1 << 2,
    COMPONENT_FOLLOW = 1 << 3,
    COMPONENT_COLLISION = 1 << 4,
    COMPONENT_PROJECTILE = 1 << 5,
    COMPONENT_VELOCITY = 1 << 6,
    COMPONENT_LIFETIME = 1 << 7,
    COMPONENT_HEALTH = 1 << 8,
    COMPONENT_DAMAGE = 1 << 9,
} ComponentType;

typedef struct {
    bool alive[MAX_ENTITIES];         // Entity existence tracking
    uint32_t component_masks[MAX_ENTITIES]; // Component presence
    uint32_t entity_count;            // Active entities
} Registry;

extern Registry registry;

typedef struct {
    float current_health;
    float max_health;
} HealthComponent;

typedef struct {
    float damage_amount;
} DamageComponent;

/*
  Create entity_get_component and entity_set_component
 */
#define ECS_COMPONENT_ACCESSORS(name, comp_type, comp_enum) \
    comp_type* entity_get_##name(Entity e) { \
        void* ptr = ecs_get_component(e, comp_enum); \
        return (comp_type*)ptr; \
    } \
    void entity_set_##name(Entity e, comp_type component) { \
        ecs_set_component(e, comp_enum, &component); \
    }

void ecs_init();

Entity entity_create();
void entity_destroy(Entity e);
bool entity_is_alive(Entity e);

void follow_system(float delta_time);
void render_system(int width, int height);

void* ecs_get_component(Entity e, ComponentType type);
void ecs_set_component(Entity e, ComponentType type, void* component);

// TODO: Move health and damage components to other game logic stuff
HealthComponent* entity_get_health(Entity e);
void entity_set_health(Entity e, HealthComponent component);
DamageComponent* entity_get_damage(Entity e);
void entity_set_damage(Entity e, DamageComponent component);
