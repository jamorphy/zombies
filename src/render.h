#pragma once

#include "ecs.h"
#include "../libs/linmath/linmath.h"

typedef struct {
    sg_buffer vertex_buffer;
    sg_buffer index_buffer;
    sg_pipeline pipeline;
    int index_count;
    // TODO: texture handles or Material component
} RenderComponent;

extern sg_shader cube_shader;
extern sg_pipeline cube_pipeline;

RenderComponent* entity_get_render(Entity e);
void entity_set_render(Entity e, RenderComponent component);

RenderComponent create_cube_render_component(void);
