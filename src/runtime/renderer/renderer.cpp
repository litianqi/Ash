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
    create_shadow_map();

    sampler_linear = context->createSampler(
        {
            .mipMap = lvk::SamplerMip_Linear,
            .wrapU = lvk::SamplerWrap_Repeat,
            .wrapV = lvk::SamplerWrap_Repeat,
            .debugName = "Sampler: linear",
        },
        nullptr);
    sampler_shadow = context->createSampler(
        {
            .wrapU = lvk::SamplerWrap_Clamp,
            .wrapV = lvk::SamplerWrap_Clamp,
            .depthCompareOp = lvk::CompareOp_LessEqual,
            .depthCompareEnabled = true,
            .debugName = "Sampler: shadow",
        },
        nullptr);
}

void Renderer::resize(uint32_t new_width, uint32_t new_height)
{
    width = new_width;
    height = new_height;
    create_depth_buffer();
    create_shadow_map();
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

void Renderer::create_shadow_map()
{
    auto* context = Device::get()->get_context();

    const uint32_t w = 4096;
    const uint32_t h = 4096;
    lvk::TextureDesc depth_desc = {
        .type = lvk::TextureType_2D,
        .format = SHADOW_MAP_FORMAT,
        .dimensions = {w, h},
        .usage = lvk::TextureUsageBits_Attachment | lvk::TextureUsageBits_Sampled,
        .numMipLevels = lvk::calcNumMipLevels(w, h),
        .debugName = "Shadow map",
    };
    shadow_map = context->createTexture(depth_desc);
    shadow_framebuffer = {
        .depthStencil = {.texture = shadow_map},
    };
}
} // namespace ash
