#pragma once

#include <memory>
#include <filesystem>
#include "LVK.h"
#include "spdlog/spdlog.h"
#include "HelpersImGui.h"

namespace fs = std::filesystem;
struct SDL_Window;
namespace lvk
{
class IContext;
}

namespace ash
{
class BaseApp
{
  public:
    BaseApp() = default;
    virtual ~BaseApp() = default;

    // The startup method will be invoked once at the start of the application.
    virtual void startup();

    // The cleanup method will be invoked once at the end of the application.
    virtual void cleanup();

    // Returns true to exit the application.
    [[nodiscard]]
    virtual bool is_done() const
    {
        return false;
    }

    // The update method will be invoked once per frame.
    virtual void update(float dt) = 0;

    // The render method will be invoked once per frame.
    virtual void render() = 0;

    // The resize method will be invoked when the window is resized.
    virtual void resize(uint32_t width, uint32_t height);

    [[nodiscard]]
    fs::path get_root_dir() const
    {
        return root_dir;
    }

    [[nodiscard]]
    fs::path get_resources_dir() const
    {
        return resources_dir;
    }

  protected:
    fs::path root_dir;
    fs::path resources_dir;
    uint32_t display_width = 1920;
    uint32_t display_height = 1080;
    SDL_Window* window = nullptr;
    std::unique_ptr<lvk::IContext> context;
    std::unique_ptr<lvk::ImGuiRenderer> imgui;
};

extern BaseApp* g_app;

int run_application(BaseApp& app, int argc, char* argv[]);
} // namespace ash

#define ASH_CREATE_APPLICATION(app_class)                                                                              \
    int main(int argc, char* argv[])                                                                                   \
    {                                                                                                                  \
        app_class app;                                                                                                 \
        return ash::run_application(app, argc, argv);                                                                  \
    }
