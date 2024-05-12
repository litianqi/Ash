#include "forward_pass.h"
#include "gfx/device.h"
#include "resource/mesh_resource.h"
#include "renderer/renderer.h"
#include "core/file_utils.h"

namespace ash
{
ForwardPass::ForwardPass(lvk::IContext& context)
{
    // Create Unlit pipeline
    {
        auto unlit_vs = std::get<0>(read_shader("mesh/unlit.vert"));
        auto unlit_fs = std::get<0>(read_shader("mesh/unlit.frag"));
        unlit_vert = context.createShaderModule({unlit_vs.c_str(), lvk::Stage_Vert, "Shader Module: main (vert)"});
        unlit_frag = context.createShaderModule({unlit_fs.c_str(), lvk::Stage_Frag, "Shader Module: main (frag)"});
        const lvk::VertexInput vdesc = {
            .attributes =
                {
                    {.location = 0, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, position)},
                    {.location = 1, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, normal)},
                    {.location = 2, .format = lvk::VertexFormat::Float2, .offset = offsetof(Vertex, uv)},
                },
            .inputBindings = {{.stride = sizeof(Vertex)}},
        };
        unlit_opaque_pipeline = context.createRenderPipeline(
            {
                .vertexInput = vdesc,
                .smVert = unlit_vert,
                .smFrag = unlit_frag,
                .color =
                    {
                        {.format = context.getSwapchainFormat()},
                    },
                .depthFormat = Renderer::DEPTH_FORMAT,
                .cullMode = lvk::CullMode_Back,
                .frontFaceWinding = lvk::WindingMode_CCW,
                .debugName = "Pipeline: mesh",
            },
            nullptr);
        unlit_transparent_pipeline = context.createRenderPipeline(
            {
                .vertexInput = vdesc,
                .smVert = unlit_vert,
                .smFrag = unlit_frag,
                .color =
                    {
                        {
                            .format = context.getSwapchainFormat(),
                            .blendEnabled = true,
                            .srcRGBBlendFactor = lvk::BlendFactor_One,
                            .srcAlphaBlendFactor = lvk::BlendFactor_One,
                            .dstRGBBlendFactor = lvk::BlendFactor_DstAlpha,
                            .dstAlphaBlendFactor = lvk::BlendFactor_Zero,
                        },
                    },
                .depthFormat = Renderer::DEPTH_FORMAT,
                .cullMode = lvk::CullMode_Back,
                .frontFaceWinding = lvk::WindingMode_CCW,
                .debugName = "Pipeline: mesh",
            },
            nullptr);
    }

    // Create Simple Lit pipeline
    {
        auto simple_lit_vs = std::get<0>(read_shader("mesh/simple_lit.vert"));
        auto simple_lit_fs = std::get<0>(read_shader("mesh/simple_lit.frag"));
        simple_lit_vert =
            context.createShaderModule({simple_lit_vs.c_str(), lvk::Stage_Vert, "Shader Module: main (vert)"});
        simple_lit_frag =
            context.createShaderModule({simple_lit_fs.c_str(), lvk::Stage_Frag, "Shader Module: main (frag)"});
        const lvk::VertexInput vdesc = {
            .attributes =
                {
                    {.location = 0, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, position)},
                    {.location = 1, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, normal)},
                    {.location = 2, .format = lvk::VertexFormat::Float2, .offset = offsetof(Vertex, uv)},
                },
            .inputBindings = {{.stride = sizeof(Vertex)}},
        };
        simple_lit_opaque_pipeline = context.createRenderPipeline(
            {
                .vertexInput = vdesc,
                .smVert = simple_lit_vert,
                .smFrag = simple_lit_frag,
                .color =
                    {
                        {.format = context.getSwapchainFormat()},
                    },
                .depthFormat = Renderer::DEPTH_FORMAT,
                .cullMode = lvk::CullMode_Back,
                .frontFaceWinding = lvk::WindingMode_CCW,
                .debugName = "Pipeline: mesh",
            },
            nullptr);
        simple_lit_transparent_pipeline = context.createRenderPipeline(
            {
                .vertexInput = vdesc,
                .smVert = simple_lit_vert,
                .smFrag = simple_lit_frag,
                .color =
                    {
                        {
                            .format = context.getSwapchainFormat(),
                            .blendEnabled = true,
                            .srcRGBBlendFactor = lvk::BlendFactor_One,
                            .srcAlphaBlendFactor = lvk::BlendFactor_One,
                            .dstRGBBlendFactor = lvk::BlendFactor_DstAlpha,
                            .dstAlphaBlendFactor = lvk::BlendFactor_Zero,
                        },
                    },
                .depthFormat = Renderer::DEPTH_FORMAT,
                .cullMode = lvk::CullMode_Back,
                .frontFaceWinding = lvk::WindingMode_CCW,
                .debugName = "Pipeline: mesh",
            },
            nullptr);
    }
}

void ForwardPass::render(const RenderPassContext& context, const PassData& data)
{
    ZoneScoped;

    // Alloc pass uniforms
    GlobalUniforms global_uniforms_data = {
        .proj = data.proj,
        .view = data.view,
        .sampler = data.sampler.index(),
        .ambient_light = data.ambient_light,
        .lights_num = std::min((uint32_t)data.lights.size(), MAX_LIGHT_COUNT),
    };
    for (uint32_t i = 0; i < data.lights.size() && i < MAX_LIGHT_COUNT; i++)
    {
        global_uniforms_data.lights[i] = data.lights[i];
    }
    auto global_uniforms = context.temp_buffer.alloc(global_uniforms_data);

    // Render Opaque List
    if (data.opaque.objects.size() > 0)
    {
        ZoneScopedN("Render Opaque");
        switch (data.shader_type)
        {
        case ShaderType::UNLIT:
            context.cmd.cmdBindRenderPipeline(unlit_opaque_pipeline);
            break;
        case ShaderType::SIMPLE_LIT:
            context.cmd.cmdBindRenderPipeline(simple_lit_opaque_pipeline);
            break;
        }
        context.cmd.cmdBindViewport(context.get_viewport());
        context.cmd.cmdBindScissorRect(context.get_scissor());
        context.cmd.cmdPushDebugGroupLabel("Render Opaque", 0xff0000ff);
        lvk::DepthState depth_state = {.compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true};
        context.cmd.cmdBindDepthState(depth_state);

        // Alloc object uniforms
        std::vector<mat4> object_uniforms_data;
        object_uniforms_data.reserve(data.opaque.objects.size());
        for (const auto& object : data.opaque.objects)
        {
            object_uniforms_data.push_back(object.transform);
        }
        auto object_uniforms = context.temp_buffer.alloc(object_uniforms_data.data(),
                                                         object_uniforms_data.size() * sizeof(ObjectUniforms));

        // Draw
        lvk::BufferHandle last_vertex_buffer;
        lvk::BufferHandle last_index_buffer;
        for (uint32_t i = 0; i != data.opaque.objects.size(); i++)
        {
            auto& object = data.opaque.objects[i];
            if (object.vertex_buffer != last_vertex_buffer)
            {
                last_vertex_buffer = object.vertex_buffer;
                context.cmd.cmdBindVertexBuffer(0, object.vertex_buffer);
            }
            if (object.index_buffer != last_index_buffer)
            {
                last_index_buffer = object.index_buffer;
                context.cmd.cmdBindIndexBuffer(object.index_buffer, lvk::IndexFormat_UI32);
            }
            auto bindings = PushConstants{
                .per_frame = global_uniforms,
                .per_object = object_uniforms + i * sizeof(ObjectUniforms),
                .material = object.material,
            };
            context.cmd.cmdPushConstants(bindings);
            context.cmd.cmdDrawIndexed(object.index_count, 1, object.index_offset);
        }
        context.cmd.cmdPopDebugGroupLabel();
    }

    // Render Transparent List
    if (data.transparent.objects.size() > 0)
    {
        ZoneScopedN("Render Transparent");
        switch (data.shader_type)
        {
        case ShaderType::UNLIT:
            context.cmd.cmdBindRenderPipeline(unlit_transparent_pipeline);
            break;
        case ShaderType::SIMPLE_LIT:
            context.cmd.cmdBindRenderPipeline(simple_lit_transparent_pipeline);
            break;
        }
        context.cmd.cmdBindViewport(context.get_viewport());
        context.cmd.cmdBindScissorRect(context.get_scissor());
        context.cmd.cmdPushDebugGroupLabel("Render Transparent", 0xff0000ff);
        lvk::DepthState depth_state = {.compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = false};
        context.cmd.cmdBindDepthState(depth_state);

        // Alloc object uniforms
        std::vector<mat4> object_uniforms_data;
        object_uniforms_data.reserve(data.transparent.objects.size());
        for (const auto& object : data.transparent.objects)
        {
            object_uniforms_data.push_back(object.transform);
        }
        auto object_uniforms = context.temp_buffer.alloc(object_uniforms_data.data(),
                                                         object_uniforms_data.size() * sizeof(ObjectUniforms));

        // Draw
        lvk::BufferHandle last_vertex_buffer;
        lvk::BufferHandle last_index_buffer;
        for (uint32_t i = 0; i != data.transparent.objects.size(); i++)
        {
            auto& object = data.transparent.objects[i];
            if (object.vertex_buffer != last_vertex_buffer)
            {
                last_vertex_buffer = object.vertex_buffer;
                context.cmd.cmdBindVertexBuffer(0, object.vertex_buffer);
            }
            if (object.index_buffer != last_index_buffer)
            {
                last_index_buffer = object.index_buffer;
                context.cmd.cmdBindIndexBuffer(object.index_buffer, lvk::IndexFormat_UI32);
            }
            auto bindings = PushConstants{
                .per_frame = global_uniforms,
                .per_object = object_uniforms + i * sizeof(ObjectUniforms),
                .material = object.material,
            };
            context.cmd.cmdPushConstants(bindings);
            context.cmd.cmdDrawIndexed(object.index_count, 1, object.index_offset);
        }
        context.cmd.cmdPopDebugGroupLabel();
    }
}
} // namespace ash
