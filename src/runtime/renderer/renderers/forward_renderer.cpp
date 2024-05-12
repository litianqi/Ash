#include "forward_renderer.h"
#include "gfx/device.h"
#include "resource/mesh_resource.h"
#include "world/components/mesh_component.h"
#include "world/world.h"
#include "world/components/camera_component.h"
#include "world/components/light_component.h"

namespace ash
{
ForwardRenderer::ForwardRenderer(Device& device, const RendererDesc& desc) : Renderer(device, desc)
{
    forward_pass = std::make_unique<ForwardPass>(*device.get_context());
}

void ForwardRenderer::render(const World* world, const CameraComponent* camera)
{
    ZoneScoped;

    temp_buffer->advance();

    // TODO: Add culling

    RenderList opaque;
    RenderList transparent;
    std::vector<GpuLight> lights;
    {
        ZoneScopedN("Collect render objects");
        for (auto& go : world->get_game_objects())
        {
            if (go.has_component<MeshComponent>())
            {
                auto& mesh = go.get_component<MeshComponent>()->mesh;
                for (auto& sub_mesh : mesh->sub_meshes)
                {
                    
                    auto render_object = RenderObject{.vertex_buffer = mesh->vertex_buffer,
                                                      .index_buffer = mesh->index_buffer,
                                                      .index_offset = sub_mesh.index_offset,
                                                      .index_count = sub_mesh.index_count,
                                                      .bounds = sub_mesh.bounds,
                                                      .material = sub_mesh.material->uniform_buffer.get_gpu_address(),
                                                      .transform = go.get_matrix()};
                    if (sub_mesh.material->alpha_mode == AlphaMode::BLEND)
                    {
                        transparent.objects.push_back(render_object);
                    }
                    else // OPAQUE && MASK
                    {
                        opaque.objects.push_back(render_object);
                    }
                }
            }
            if (go.has_component<LightComponent>())
            {
                lights.push_back(go.get_component<LightComponent>()->get_gpu_light());
            }
        }
    }
    {
        ZoneScopedN("Sort render objects");
        opaque.sort(&RenderList::opaque_sort);
        // TODO: sort transparent
    }

    auto* context = Device::get()->get_context();

    lvk::TextureHandle swapchain_texture = context->getCurrentSwapchainTexture();
    lvk::Framebuffer framebuffer = {.color = {{.texture = swapchain_texture}},
                                    .depthStencil = {.texture = depth_buffer}};

    // Command buffers (1-N per thread): create, submit and forget
    lvk::ICommandBuffer& cmd = context->acquireCommandBuffer();

    // This will clear the framebuffer
    lvk::RenderPass render_pass = {
        .color = {{
            .loadOp = lvk::LoadOp_Clear,
            .storeOp = lvk::StoreOp_Store,
            .clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
        }},
        .depth = {.loadOp = lvk::LoadOp_Clear, .storeOp = lvk::StoreOp_Store, .clearDepth = 1.0}};
    cmd.cmdBeginRendering(render_pass, framebuffer);
    {
        auto pass_context = RenderPassContext{
            .cmd = cmd,
            .temp_buffer = *temp_buffer,
            .width = width,
            .height = height,
        };
        auto pass_data = ForwardPass::PassData{
            .proj = camera->get_projection_matrix(),
            .view = camera->get_view_matrix(),
            .sampler = sampler,
            .shader_type = shader_type,
            .opaque = opaque,
            .transparent = transparent,
            .lights = lights,
        };
        forward_pass->render(pass_context, pass_data);
        auto* imgui = Device::get()->get_imgui();
        imgui->end_frame(cmd, framebuffer);
    }
    cmd.cmdEndRendering();

    context->submit(cmd, swapchain_texture);
}

} // namespace ash
