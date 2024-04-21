#include "app.h"
#include "SDL3/SDL.h"
#include "spdlog/spdlog.h"
#include "minilog/minilog.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "input/input_manager.h"
#include "gfx/device.h"

namespace ash
{
static BaseApp* s_app;

BaseApp* BaseApp::get()
{
    return s_app;
}

void BaseApp::startup()
{
    s_app = this;

    minilog::initialize(nullptr, {.logLevelPrintToConsole = minilog::Warning, .threadNames = false});

    fs::path dir = fs::current_path();
    const char* resources_dir_name = "resources";
    while (dir != fs::current_path().root_path() && !exists(dir / fs::path(resources_dir_name)))
    {
        dir = dir.parent_path();
    }
    root_dir = dir;
    resources_dir = dir / fs::path(resources_dir_name);
    spdlog::info("Root dir = {}", root_dir.string());
    spdlog::info("Resources dir = {}", resources_dir.string());

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

    add_subsystem<InputManager>();
    add_subsystem<Device>(window, display_width, display_height);
}

void BaseApp::cleanup()
{
    remove_subsystem<Device>();
    remove_subsystem<InputManager>();
    SDL_DestroyWindow(window);
    SDL_Quit();
    window = nullptr;
}

void BaseApp::resize(uint32_t width, uint32_t height)
{
    display_width = width;
    display_height = height;
    get_subsystem<Device>()->resize(width, height);
    spdlog::info("Resized to {} x {}", width, height);
}

int run_application(BaseApp& app, int argc, char* argv[])
{
    app.startup();

    bool close_requested = false;
    bool stop_rendering = false;
    const uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t last_time = SDL_GetPerformanceCounter();
    
    while (!app.is_done() && !close_requested)
    {
        InputManager::get()->tick();

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
                    InputManager::get()->on_mouse_down(static_cast<MouseButton>(button));
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
                    InputManager::get()->on_mouse_up(static_cast<MouseButton>(button));
                }
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                ImGui::GetIO().MousePos = ImVec2(event.button.x, event.button.y);
                InputManager::get()->on_mouse_move({event.button.x, event.button.y});
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                ImGui::GetIO().MouseWheelH = event.wheel.x;
                ImGui::GetIO().MouseWheel = event.wheel.y;
                InputManager::get()->on_mouse_scroll({event.wheel.x, event.wheel.y});
                break;
            }
            case SDL_EVENT_KEY_DOWN: {
                InputManager::get()->on_key_down(static_cast<KeyCode>(event.key.keysym.sym));
                break;
            }
            case SDL_EVENT_KEY_UP: {
                InputManager::get()->on_key_up(static_cast<KeyCode>(event.key.keysym.sym));
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
