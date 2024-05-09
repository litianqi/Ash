#include "renderer.h"
#include "gfx/device.h"
#include "render_types.h"

namespace ash
{
Renderer::Renderer(Device& device, const RendererDesc& desc) : width(desc.width), height(desc.height)
{
    auto* context = device.get_context();

    temp_buffer = std::make_unique<BufferRing>(context, 3, desc.temp_buffer_size, "Temp Buffer");

    SWAPCHAIN_FORMAT = context->getSwapchainFormat();

    create_depth_buffer();

    sampler = context->createSampler({.debugName = "Sampler: linear"}, nullptr);
}

void Renderer::resize(uint32_t new_width, uint32_t new_height)
{
    width = new_width;
    height = new_height;
    create_depth_buffer();
}

void Renderer::create_depth_buffer()
{
    auto* context = Device::get()->get_context();

    lvk::TextureDesc depth_desc = {
        .type = lvk::TextureType_2D,
        .format = DEPTH_FORMAT,
        .dimensions = {width, height},
        .usage = lvk::TextureUsageBits_Attachment | lvk::TextureUsageBits_Sampled,
        .numMipLevels = lvk::calcNumMipLevels(width, height),
        .debugName = "Offscreen framebuffer (d)",
    };
    depth_buffer = context->createTexture(depth_desc);
}
} // namespace ash
