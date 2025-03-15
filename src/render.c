#include "render.h"
#include "../libs/sokol/HandmadeMath.h"
#include "cube.glsl.h"

#include <stddef.h>

ECS_COMPONENT_ACCESSORS(render, RenderComponent, COMPONENT_RENDER)

sg_shader cube_shader = {0};
sg_pipeline cube_pipeline = {0};
bool render_initialized = false;

RenderComponent create_render_component(
    const float* vertices, size_t vertex_size,
    const uint16_t* indices, size_t index_count
)
{
    if (!render_initialized) {
        cube_shader = sg_make_shader(cube_shader_desc(sg_query_backend()));
        cube_pipeline = sg_make_pipeline(&(sg_pipeline_desc){
            .shader = cube_shader,
            .layout = {
                .attrs[0] = { .format = SG_VERTEXFORMAT_FLOAT3 }, // Position
                .attrs[1] = { .format = SG_VERTEXFORMAT_FLOAT4 }, // Color
            },
            .index_type = SG_INDEXTYPE_UINT16,
            .cull_mode = SG_CULLMODE_FRONT,
            .depth = {
                .compare = SG_COMPAREFUNC_LESS_EQUAL,
                .write_enabled = true
            },
            .label = "cube-pipeline"
        });
        render_initialized = true;
    }

    RenderComponent rc = {0};
    rc.vertex_buffer = sg_make_buffer(&(sg_buffer_desc){
        .data = { .ptr = vertices, .size = vertex_size },
        .label = "vertices"
    });
    rc.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = { .ptr = indices, .size = index_count * sizeof(uint16_t) },
        .label = "indices"
    });
    rc.index_count = index_count;
    rc.pipeline = cube_pipeline;

    return rc;
}
