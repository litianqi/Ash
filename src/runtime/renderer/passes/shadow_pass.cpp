#include "shadow_pass.h"
#include "gfx/device.h"
#include "resource/mesh_resource.h"
#include "renderer/renderer.h"
#include "core/file_utils.h"

namespace ash
{
struct alignas(16) ShadowPassUniforms
{
    mat4 light;
};

struct alignas(16) ShadowPassPushConstants
{
    uint64_t per_frame = 0;
    uint64_t per_object = 0;
};

ShadowPass::ShadowPass(lvk::IContext& context)
{
    auto shadow_vs = std::get<0>(read_shader("shadow.vert"));
    auto shadow_fs = std::get<0>(read_shader("shadow.frag"));
    shadow_vert = context.createShaderModule({shadow_vs.c_str(), lvk::Stage_Vert, "Shader Module: main (vert)"});
    shadow_frag = context.createShaderModule({shadow_fs.c_str(), lvk::Stage_Frag, "Shader Module: main (frag)"});
    const lvk::VertexInput vdesc = {
        .attributes =
            {
                {.location = 0, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, position)},
                {.location = 1, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, normal)},
                {.location = 2, .format = lvk::VertexFormat::Float2, .offset = offsetof(Vertex, uv)},
            },
        .inputBindings = {{.stride = sizeof(Vertex)}},
    };
    shadow_opaque_pipeline = context.createRenderPipeline(
        {
            .vertexInput = vdesc,
            .smVert = shadow_vert,
            .smFrag = shadow_frag,
            .depthFormat = Renderer::SHADOW_MAP_FORMAT,
            .cullMode = lvk::CullMode_None,
            .debugName = "Pipeline: shadow",
        },
        nullptr);
    shadow_transparent_pipeline = context.createRenderPipeline(
        {
            .vertexInput = vdesc,
            .smVert = shadow_vert,
            .smFrag = shadow_frag,
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

void ShadowPass::render(const RenderPassContext& context, const PassData& data)
{
    ZoneScoped;

    // Alloc pass uniforms
    ShadowPassUniforms global_uniforms_data = {
        .light = data.light,
    };
    auto global_uniforms = context.temp_buffer.alloc(global_uniforms_data);

    // Render Opaque List
    if (data.opaque.objects.size() > 0)
    {
        ZoneScopedN("Render Opaque Shadow");
        context.cmd.cmdBindRenderPipeline(shadow_opaque_pipeline);
        context.cmd.cmdPushDebugGroupLabel("Render Opaque Shadow", 0xff0000ff);
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
            auto bindings = ShadowPassPushConstants{
                .per_frame = global_uniforms,
                .per_object = object_uniforms + i * sizeof(ObjectUniforms),
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
        context.cmd.cmdBindRenderPipeline(shadow_transparent_pipeline);
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
            auto bindings = ShadowPassPushConstants{
                .per_frame = global_uniforms,
                .per_object = object_uniforms + i * sizeof(ObjectUniforms),
            };
            context.cmd.cmdPushConstants(bindings);
            context.cmd.cmdDrawIndexed(object.index_count, 1, object.index_offset);
        }
        context.cmd.cmdPopDebugGroupLabel();
    }
}
} // namespace ash
