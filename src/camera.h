#pragma once

#include "camera.h"
#include "ecs.h"
#include "../libs/linmath/linmath.h"

typedef struct {
    float fov;         // Field of view in degrees (e.g. 45.0f)
    float aspect;      // Aspect ratio (width/height)
    float near_plane;  // Near clip plane
    float far_plane;   // Far clip plane
    float pitch;
    float yaw;
} CameraComponent;

typedef struct {
    Entity target;
    vec3 offset;
} FollowComponent;

CameraComponent* entity_get_camera(Entity e);
void entity_set_camera(Entity e, CameraComponent component);

FollowComponent* entity_get_follow(Entity e);
void entity_set_follow(Entity e, FollowComponent);
