#pragma once

#include "ecs.h"
#include "../libs/linmath/linmath.h"

typedef struct {
    vec3 position;
    quat rotation;
    vec3 scale;
    bool dirty;
} TransformComponent;

// Macro generated
TransformComponent* entity_get_transform(Entity e);
void entity_set_transform(Entity e, TransformComponent component);
