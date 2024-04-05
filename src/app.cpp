#include "app.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "spdlog/spdlog.h"
#include "minilog/minilog.h"
#include "vulkan/VulkanClasses.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "device/input_manager.h"

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
BaseApp* g_app;

void BaseApp::startup()
{
    minilog::initialize(nullptr, {.logLevelPrintToConsole = minilog::Warning, .threadNames = false});

    fs::path dir = fs::current_path();
    const char* content_dir_name = "content";
    while (dir != fs::current_path().root_path() && !exists(dir / fs::path(content_dir_name)))
    {
        dir = dir.parent_path();
    }
    root_dir = dir;
    content_dir = dir / fs::path(content_dir_name);
    spdlog::info("Root dir = {}", root_dir.string());
    spdlog::info("Content dir = {}", content_dir.string());

    // init the library, here we make a window so we only need the Video
    // capabilities.
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        spdlog::error("Failed to init, error: {}", SDL_GetError());
        exit(1);
    }

    // create a window
    window = SDL_CreateWindow("Ash", display_width, display_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (!window)
    {
        spdlog::error("Failed to create window, error: {}", SDL_GetError());
        exit(1);
    }

    spdlog::info("Application started successfully!");

    context = create_vulkan_context_with_swapchain(window, display_width, display_height, {});
    imgui = std::make_unique<lvk::ImGuiRenderer>(*context);
}

void BaseApp::cleanup()
{
    imgui = nullptr;
    context = nullptr;
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void BaseApp::resize(uint32_t width, uint32_t height)
{
    display_width = width;
    display_height = height;
    context->recreateSwapchain(static_cast<int>(width), static_cast<int>(height));
    spdlog::info("Resized to {} x {}", width, height);
}

int run_application(BaseApp& app, int argc, char* argv[])
{
    g_app = &app;

    app.startup();

    bool close_requested = false;
    bool stop_rendering = false;
    const uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t last_time = SDL_GetPerformanceCounter();
    
    while (!app.is_done() && !close_requested)
    {
        InputManager::get().tick();

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
                app.resize(event.window.data1, event.window.data2);
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
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                const int32_t button = event.button.button;
                if (button > 0)
                {
                    if (button == SDL_BUTTON_LEFT)
                    {
                        ImGui::GetIO().MouseDown[ImGuiMouseButton_::ImGuiMouseButton_Left] = true;
                    }
                    else if (button == SDL_BUTTON_RIGHT)
                    {
                        ImGui::GetIO().MouseDown[ImGuiMouseButton_::ImGuiMouseButton_Right] = true;
                    }
                    else if (button == SDL_BUTTON_MIDDLE)
                    {
                        ImGui::GetIO().MouseDown[ImGuiMouseButton_::ImGuiMouseButton_Middle] = true;
                    }
                    InputManager::get().on_mouse_down(static_cast<MouseButton>(button));
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                const int32_t button = event.button.button;
                if (button > 0)
                {
                    if (button == SDL_BUTTON_LEFT)
                    {
                        ImGui::GetIO().MouseDown[ImGuiMouseButton_::ImGuiMouseButton_Left] = false;
                    }
                    else if (button == SDL_BUTTON_RIGHT)
                    {
                        ImGui::GetIO().MouseDown[ImGuiMouseButton_::ImGuiMouseButton_Right] = false;
                    }
                    else if (button == SDL_BUTTON_MIDDLE)
                    {
                        ImGui::GetIO().MouseDown[ImGuiMouseButton_::ImGuiMouseButton_Middle] = false;
                    }
                    InputManager::get().on_mouse_up(static_cast<MouseButton>(button));
                }
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                ImGui::GetIO().MousePos = ImVec2(event.button.x, event.button.y);
                InputManager::get().on_mouse_move({event.button.x, event.button.y});
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                ImGui::GetIO().MouseWheelH = event.wheel.x;
                ImGui::GetIO().MouseWheel = event.wheel.y;
                InputManager::get().on_mouse_scroll({event.wheel.x, event.wheel.y});
                break;
            }
            case SDL_EVENT_KEY_DOWN: {
                InputManager::get().on_key_down(event.key.keysym.sym);
                break;
            }
            case SDL_EVENT_KEY_UP: {
                InputManager::get().on_key_up(event.key.keysym.sym);
                break;
            }
            }
        }

        uint64_t current_time = SDL_GetPerformanceCounter();
        float dt = static_cast<float>(current_time - last_time) / static_cast<float>(freq);
        last_time = current_time;
        app.update(dt);

        // do not draw if we are minimized
        if (stop_rendering)
        {
            // throttle the speed to avoid the endless spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        app.render();
    }

    app.cleanup();
    return 0;
}
} // namespace ash
