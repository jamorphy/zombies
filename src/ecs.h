#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "linmath.h"

#include "../libs/sokol/sokol_gfx.h"
#include "../libs/sokol/sokol_glue.h"
#include "../libs/linmath/linmath.h"

#include "constants.h"
#include "types.h"

typedef uint32_t Entity;
#define INVALID_ENTITY MAX_ENTITIES

typedef enum ComponentType {
    COMPONENT_NONE = 0,
    COMPONENT_TRANSFORM = 1 << 0,
    COMPONENT_RENDER = 1 << 1,
    COMPONENT_CAMERA = 1 << 2
} ComponentType;

typedef struct {
    bool alive[MAX_ENTITIES];         // Entity existence tracking
    uint32_t component_masks[MAX_ENTITIES]; // Component presence
    uint32_t entity_count;            // Active entities
} Registry;

typedef struct {
    vec3 position;
    quat rotation;
    vec3 scale;
} TransformComponent;

typedef struct {
    sg_buffer vertex_buffer;
    sg_buffer index_buffer;
    sg_pipeline pipeline;
    int index_count;
    // TODO: texture handles or Material component
} RenderComponent;

typedef struct {
    float fov;         // Field of view in degrees (e.g. 45.0f)
    float aspect;      // Aspect ratio (width/height)
    float near_plane;  // Near clip plane
    float far_plane;   // Far clip plane
    float pitch;
    float yaw;
} CameraComponent;

void ecs_init();

Entity entity_create();
void entity_destroy(Entity e);
bool entity_is_alive(Entity e);

TransformComponent* entity_get_transform(Entity e);
void entity_add_transform(Entity e, vec3 pos, quat rot, vec3 scale);
void entity_add_render(Entity e, RenderComponent component);

RenderComponent create_cube_render_component(void);

void entity_add_camera(Entity e, float fov, float aspect, float near_plane, float far_plane);
CameraComponent* entity_get_camera(Entity e);

void render_system(int width, int height);
