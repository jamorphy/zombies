#include "physics.h"
#include <string.h>

ECS_COMPONENT_ACCESSORS(collision, CollisionComponent, COMPONENT_COLLISION)

void physics_init(void) {
}

void physics_update_collision_transform(TransformComponent* transform, CollisionComponent* collision)
{
    vec3 center;
    vec3_add(center, transform->position, collision->center_offset);

    vec3 half_size;
    for (int i = 0; i < 3; i++) {
        half_size[i] = collision->size[i] * transform->scale[i];
    }

    vec3_sub(collision->min, center, half_size);
    vec3_add(collision->max, center, half_size);
}

bool physics_check_aabb_collision(const vec3 min1, const vec3 max1, const vec3 min2, const vec3 max2)
{
    return (min1[0] <= max2[0] && max1[0] >= min2[0]) &&
           (min1[1] <= max2[1] && max1[1] >= min2[1]) &&
           (min1[2] <= max2[2] && max1[2] >= min2[2]);
}

void physics_system_update(float delta_time)
{
    for (Entity e = 0; e < MAX_ENTITIES; e++) {
        if (!entity_is_alive(e)) continue;
        if ((registry.component_masks[e] & (COMPONENT_TRANSFORM | COMPONENT_COLLISION)) ==
            (COMPONENT_TRANSFORM | COMPONENT_COLLISION)) {
            TransformComponent* transform = entity_get_transform(e);
            CollisionComponent* collision = entity_get_collision(e);
            if (transform && collision) {
                physics_update_collision_transform(transform, collision);
            }
        }
    }

    for (Entity e1 = 0; e1 < MAX_ENTITIES; e1++) {
        if (!entity_is_alive(e1) || !(registry.component_masks[e1] & COMPONENT_COLLISION)) continue;
        CollisionComponent* c1 = entity_get_collision(e1);
        TransformComponent* t1 = entity_get_transform(e1);

        for (Entity e2 = e1 + 1; e2 < MAX_ENTITIES; e2++) {
            if (!entity_is_alive(e2) || !(registry.component_masks[e2] & COMPONENT_COLLISION)) continue;
            CollisionComponent* c2 = entity_get_collision(e2);
            TransformComponent* t2 = entity_get_transform(e2);

            if (physics_check_aabb_collision(c1->min, c1->max, c2->min, c2->max)) {
                vec3 penetration = {0};
                vec3 delta;
                vec3_sub(delta, t2->position, t1->position);

                float overlap_x = fminf(c1->max[0] - c2->min[0], c2->max[0] - c1->min[0]);
                float overlap_y = fminf(c1->max[1] - c2->min[1], c2->max[1] - c1->min[1]);
                float overlap_z = fminf(c1->max[2] - c2->min[2], c2->max[2] - c1->min[2]);

                float min_overlap = fminf(fminf(overlap_x, overlap_y), overlap_z);
                if (min_overlap == overlap_x && overlap_x > 0) {
                    penetration[0] = (delta[0] > 0) ? overlap_x : -overlap_x;
                } else if (min_overlap == overlap_y && overlap_y > 0) {
                    penetration[1] = (delta[1] > 0) ? overlap_y : -overlap_y;
                } else if (min_overlap == overlap_z && overlap_z > 0) {
                    penetration[2] = (delta[2] > 0) ? overlap_z : -overlap_z;
                }

                if (c1->is_static && !c2->is_static) {
                    vec3_add(t2->position, t2->position, penetration);
                } else if (!c1->is_static && c2->is_static) {
                    vec3 neg_penetration;
                    vec3_scale(neg_penetration, penetration, -1.0f);
                    vec3_add(t1->position, t1->position, neg_penetration);
                } else if (!c1->is_static && !c2->is_static) {
                    vec3 half_penetration;
                    vec3_scale(half_penetration, penetration, 0.5f);
                    vec3 neg_half_penetration;
                    vec3_scale(neg_half_penetration, penetration, -0.5f);
                    vec3_add(t1->position, t1->position, neg_half_penetration);
                    vec3_add(t2->position, t2->position, half_penetration);
                }

                physics_update_collision_transform(t1, c1);
                physics_update_collision_transform(t2, c2);
            }
        }
    }
}
