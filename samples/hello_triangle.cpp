#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "SDL3/SDL_vulkan.h"
#include "spdlog/spdlog.h"
#include "LVK.h"
#include "vulkan/VulkanClasses.h"

const char* code_vs = R"(
#version 460
layout (location=0) out vec3 color;
const vec2 pos[3] = vec2[3](
	vec2(-0.6, -0.4),
	vec2( 0.6, -0.4),
	vec2( 0.0,  0.6)
);
const vec3 col[3] = vec3[3](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);
void main() {
	gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
	color = col[gl_VertexIndex];
}
)";

const char* code_fs = R"(
#version 460
layout (location=0) in vec3 color;
layout (location=0) out vec4 out_FragColor;

void main() {
	out_FragColor = vec4(color, 1.0);
};
)";

int g_width = 1920;
int g_height = 1080;

SDL_Window* g_window = nullptr;

std::unique_ptr<lvk::IContext> g_vulkan_ctx;
lvk::Holder<lvk::ShaderModuleHandle> g_vert;
lvk::Holder<lvk::ShaderModuleHandle> g_frag;
lvk::Holder<lvk::RenderPipelineHandle> g_render_pipeline;

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

void init()
{
    minilog::initialize(nullptr, {.logLevelPrintToConsole = minilog::Warning, .threadNames = false});

    // init the library, here we make a window so we only need the Video
    // capabilities.
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        spdlog::error("Failed to init, error: {}", SDL_GetError());
        exit(1);
    }

    // create a window
    g_window = SDL_CreateWindow("Ash", g_width, g_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (!g_window)
    {
        spdlog::error("Failed to create window, error: {}", SDL_GetError());
        exit(1);
    }

    spdlog::info("Application started successfully!");

    g_vulkan_ctx = create_vulkan_context_with_swapchain(g_window, g_width, g_height, {});
    if (!g_vulkan_ctx)
    {
        return;
    }

    g_vert = g_vulkan_ctx->createShaderModule({code_vs, lvk::Stage_Vert, "Shader Module: main (vert)"});
    g_frag = g_vulkan_ctx->createShaderModule({code_fs, lvk::Stage_Frag, "Shader Module: main (frag)"});

    g_render_pipeline = g_vulkan_ctx->createRenderPipeline(
        {
            .smVert = g_vert,
            .smFrag = g_frag,
            .color = {{.format = g_vulkan_ctx->getSwapchainFormat()}},
        },
        nullptr);

    LVK_ASSERT(g_render_pipeline.valid());
}

void destroy()
{
    g_vert = nullptr;
    g_frag = nullptr;
    g_render_pipeline = nullptr;
    g_vulkan_ctx = nullptr;
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

void resize()
{
    if (!g_width || !g_height)
    {
        return;
    }
    spdlog::info("Window is resized to {}x{}", g_width, g_height);
    g_vulkan_ctx->recreateSwapchain(g_width, g_height);
}

void render()
{
    if (!g_width || !g_height)
    {
        return;
    }

    lvk::ICommandBuffer& buffer = g_vulkan_ctx->acquireCommandBuffer();

    // This will clear the framebuffer
    buffer.cmdBeginRendering({.color = {{.loadOp = lvk::LoadOp_Clear, .clearColor = {0.0f, 0.0f, 0.0f, 1.0f}}}},
                             {.color = {{.texture = g_vulkan_ctx->getCurrentSwapchainTexture()}}});
    buffer.cmdBindRenderPipeline(g_render_pipeline);
    buffer.cmdPushDebugGroupLabel("Render Triangle", 0xff0000ff);
    buffer.cmdDraw(3);
    buffer.cmdPopDebugGroupLabel();
    buffer.cmdEndRendering();
    g_vulkan_ctx->submit(buffer, g_vulkan_ctx->getCurrentSwapchainTexture());
}

// Note: your main function __must__ take this form, otherwise on nonstandard platforms (iOS, etc), your app will not
// launch.
int main(int argc, char* argv[])
{
    init();

    bool close_requested = false;
    bool stop_rendering = false;
    while (!close_requested)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                close_requested = true;
                break;
            }
            case SDL_EVENT_WINDOW_RESIZED: {
                g_width = event.window.data1;
                g_height = event.window.data2;
                resize();
                break;
            }
            case SDL_EVENT_WINDOW_MINIMIZED: {
                stop_rendering = true;
                break;
            }
            case SDL_EVENT_WINDOW_RESTORED: {
                stop_rendering = false;
                break;
            }
            }
        }

        // do not draw if we are minimized
        if (stop_rendering)
        {
            // throttle the speed to avoid the endless spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        render();
    }

    destroy();
    return 0;
}
