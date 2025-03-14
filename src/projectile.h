#pragma once

#include "ecs.h"

typedef struct {
    Entity owner;
} ProjectileComponent;

void projectile_init(void);
void create_projectile(Entity shooter, vec3 position, vec3 direction);
ProjectileComponent* entity_get_projectile(Entity e);
void entity_set_projectile(Entity e, ProjectileComponent component);
