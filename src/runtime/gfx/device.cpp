#include "device.h"
#include "vulkan/VulkanClasses.h"
#include "SDL3/SDL_vulkan.h"
#include "app/app.h"

namespace
{
bool init_vulkan_context_with_swapchain(std::unique_ptr<lvk::VulkanContext>& ctx, uint32_t width, uint32_t height,
                                        lvk::HWDeviceType preferred_device_type)
{
    using namespace lvk;
    HWDeviceDesc device;
    uint32_t num_devices = ctx->queryDevices(preferred_device_type, &device, 1);

    if (!num_devices)
    {
        if (preferred_device_type == HWDeviceType_Discrete)
        {
            num_devices = ctx->queryDevices(HWDeviceType_Integrated, &device);
        }
        else if (preferred_device_type == HWDeviceType_Integrated)
        {
            num_devices = ctx->queryDevices(HWDeviceType_Discrete, &device);
        }
    }

    if (!num_devices)
    {
        LVK_ASSERT_MSG(false, "GPU is not found");
        return false;
    }

    Result res = ctx->initContext(device);

    if (!res.isOk())
    {
        LVK_ASSERT_MSG(false, "Failed initContext()");
        return false;
    }

    if (width > 0 && height > 0)
    {
        res = ctx->initSwapchain(width, height);
        if (!res.isOk())
        {
            LVK_ASSERT_MSG(false, "Failed to create swapchain");
            return false;
        }
    }
    return true;
}

std::unique_ptr<lvk::IContext> create_vulkan_context_with_swapchain(
    SDL_Window* window, uint32_t width, uint32_t height, const lvk::ContextConfig& cfg,
    lvk::HWDeviceType preferred_device_type = lvk::HWDeviceType_Discrete)
{
    using namespace lvk;

    std::unique_ptr<lvk::VulkanContext> ctx;
    ctx = std::make_unique<lvk::VulkanContext>(cfg, nullptr);
    VkSurfaceKHR surface;
    bool result = SDL_Vulkan_CreateSurface(window, ctx->getVkInstance(), nullptr, &surface);
    if (!result)
    {
        spdlog::error("Failed to create Vulkan surface, error: {}", SDL_GetError());
        return nullptr;
    }
    ctx->setVkSurface(surface);
    if (!init_vulkan_context_with_swapchain(ctx, width, height, preferred_device_type))
    {
        return nullptr;
    }

    return std::move(ctx);
}
} // namespace

namespace ash
{
Device* Device::get()
{
    if (auto* app = BaseApp::get())
    {
        return app->get_subsystem<Device>();
    }
    return nullptr;
}

Device::Device(SDL_Window* window, uint32_t width, uint32_t height)
{
    context = create_vulkan_context_with_swapchain(window, width, height, {});
    imgui = std::make_unique<ImGuiRenderer>(*context);
    imgui->resize(width, height);

    const uint32_t pixel = 0xFFFFFFFF;
    white_texture = context->createTexture(
        {
            .type = lvk::TextureType_2D,
            .format = lvk::Format_R_UN8,
            .dimensions = {1, 1},
            .usage = lvk::TextureUsageBits_Sampled,
            .swizzle = {lvk::Swizzle_1, lvk::Swizzle_1, lvk::Swizzle_1, lvk::Swizzle_1},
            .data = &pixel,
            .debugName = "Texture: 1x1 white",
        },
        nullptr);
    linear_sampler = context->createSampler(
        {
            .mipMap = lvk::SamplerMip_Linear,
            .wrapU = lvk::SamplerWrap_Repeat,
            .wrapV = lvk::SamplerWrap_Repeat,
            .debugName = "Sampler: linear",
        },
        nullptr);

    material_buffer = std::make_unique<StorageBuffer>(context.get(), 12 * 1024 * 1024, "material buffer");
}

void Device::resize(uint32_t width, uint32_t height)
{
    context->recreateSwapchain(static_cast<int>(width), static_cast<int>(height));
    imgui->resize(width, height);
}
} // namespace ash
