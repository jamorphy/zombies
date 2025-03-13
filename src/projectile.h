#pragma once

#include "ecs.h"

typedef struct {
    Entity owner;
} ProjectileComponent;

void create_projectile(Entity shooter, Entity camera_entity);
ProjectileComponent* entity_get_projectile(Entity e);
void entity_set_projectile(Entity e, ProjectileComponent component);
