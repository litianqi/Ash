#include "forward_pass.h"
#include "gfx/device.h"
#include "resource/mesh_resource.h"
#include "renderer/renderer.h"
#include "core/file_utils.h"

namespace
{
struct alignas(16) GlobalUniforms
{
    mat4 proj;
    mat4 view;
    uint32_t sampler;
};

struct ObjectUniforms
{
    mat4 model;
};

struct alignas(16) PushConstants
{
    uint64_t per_frame = 0;
    uint64_t per_object = 0;
    uint64_t material = 0;
};
} // namespace

namespace ash
{
ForwardPass::ForwardPass(lvk::IContext& context)
{
    auto code_vs = std::get<0>(read_shader("forward.vert"));
    auto code_fs = std::get<0>(read_shader("forward.frag"));
    vert = context.createShaderModule({code_vs.c_str(), lvk::Stage_Vert, "Shader Module: main (vert)"});
    frag = context.createShaderModule({code_fs.c_str(), lvk::Stage_Frag, "Shader Module: main (frag)"});
    const lvk::VertexInput vdesc = {
        .attributes =
            {
                {.location = 0, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, position)},
                {.location = 1, .format = lvk::VertexFormat::Float3, .offset = offsetof(Vertex, normal)},
                {.location = 2, .format = lvk::VertexFormat::Float2, .offset = offsetof(Vertex, uv)},
            },
        .inputBindings = {{.stride = sizeof(Vertex)}},
    };
    render_pipeline = context.createRenderPipeline(
        {
            .vertexInput = vdesc,
            .smVert = vert,
            .smFrag = frag,
            .color =
                {
                    {.format = context.getSwapchainFormat()},
                },
            .depthFormat = Renderer::DEPTH_FORMAT,
            .cullMode = lvk::CullMode_Back,
            .frontFaceWinding = lvk::WindingMode_CW,
            .debugName = "Pipeline: mesh",
        },
        nullptr);
}

void ForwardPass::render(const RenderPassContext& context, const PassData& data)
{
    ZoneScoped;
    
    // Render Opaque List
    {
        ZoneScopedN("Render Opaque");
        context.cmd.cmdBindRenderPipeline(render_pipeline);
        context.cmd.cmdBindViewport(context.get_viewport());
        context.cmd.cmdBindScissorRect(context.get_scissor());
        context.cmd.cmdPushDebugGroupLabel("Render Opaque", 0xff0000ff);
        lvk::DepthState depth_state = {.compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true};
        context.cmd.cmdBindDepthState(depth_state);
        
        // Alloc pass uniforms
        GlobalUniforms global_uniforms_data = {
            .proj = data.proj,
            .view = data.view,
            .sampler = data.sampler.index(),
        };
        auto global_uniforms = context.temp_buffer.alloc(global_uniforms_data);
        
        // Alloc object uniforms
        std::vector<mat4> object_uniforms_data;
        object_uniforms_data.reserve(data.opaque.objects.size());
        for (const auto& object : data.opaque.objects)
        {
            object_uniforms_data.push_back(object.transform);
        }
        auto object_uniforms = context.temp_buffer.alloc(object_uniforms_data.data(), object_uniforms_data.size() * sizeof(ObjectUniforms));
        
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
    }
    
    // TODO: Render Transparent List
}
} // namespace ash
