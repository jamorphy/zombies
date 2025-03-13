#include "render.h"
#include "../libs/sokol/HandmadeMath.h"
#include "cube.glsl.h"

ECS_COMPONENT_ACCESSORS(render, RenderComponent, COMPONENT_RENDER)

sg_shader cube_shader = {0};
sg_pipeline cube_pipeline = {0};
bool render_initialized = false;

RenderComponent create_cube_render_component(void)
{
    static float vertices[] = {
        // positions      // colors (RGBA)
        // front
        -1, -1,  1,   1,0,0,1,
         1, -1,  1,   0,1,0,1,
         1,  1,  1,   0,0,1,1,
        -1,  1,  1,   1,1,0,1,
        // back
        -1, -1, -1,   1,0,1,1,
         1, -1, -1,   0,1,1,1,
         1,  1, -1,   0.5f,0.5f,0.5f,1,
        -1,  1, -1,   0,0,0,1
    };
    static uint16_t indices[] = {
        // front
        0,1,2,  0,2,3,
        // right
        1,5,6,  1,6,2,
        // back
        5,4,7,  5,7,6,
        // left
        4,0,3,  4,3,7,
        // bottom
        4,5,1,  4,1,0,
        // top
        3,2,6,  3,6,7
    };

    // Initialize shader and pipeline once
    if (!render_initialized) {
        cube_shader = sg_make_shader(cube_shader_desc(sg_query_backend()));
        cube_pipeline = sg_make_pipeline(&(sg_pipeline_desc){
            .shader = cube_shader,
            .layout = {
                .attrs[0] = { .format = SG_VERTEXFORMAT_FLOAT3 },
                .attrs[1] = { .format = SG_VERTEXFORMAT_FLOAT4 },
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
        .data = SG_RANGE(vertices),
        .label = "cube-vertices"
    });
    rc.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
        .label = "cube-indices"
    });
    rc.index_count = sizeof(indices) / sizeof(indices[0]);
    rc.pipeline = cube_pipeline; // Reuse the static pipeline

    return rc;
}
