#pragma once

#include <memory>
#include "LVK.h"
#include "spdlog/spdlog.h"

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

    virtual void startup();

    virtual void cleanup();

    [[nodiscard]] virtual bool is_done() const;

    // The update method will be invoked once per frame.
    virtual void update(float dt) = 0;

    virtual void render() = 0;

    virtual void resize(uint32_t width, uint32_t height);

  protected:
    uint32_t display_width = 1920;
    uint32_t display_height = 1080;
    SDL_Window* window = nullptr;
    bool close_requested = false;
    std::unique_ptr<lvk::IContext> context;
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
