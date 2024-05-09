#pragma once

#include <memory>
#include <filesystem>
#include <typeindex>
#include "spdlog/spdlog.h"
#include "app_subsystem.h"

namespace fs = std::filesystem;
struct SDL_Window;

namespace ash
{
class BaseApp
{
  public:
    static BaseApp* get();

    BaseApp() = default;
    virtual ~BaseApp() = default;

    // The startup method will be invoked once at the start of the application.
    virtual void startup();

    // The cleanup method will be invoked once at the end of the application.
    virtual void cleanup();

    // Returns true to exit the application.
    virtual bool is_done() const
    {
        return false;
    }

    // The update method will be invoked once per frame.
    virtual void update(float dt) = 0;

    // The render method will be invoked once per frame.
    virtual void render() = 0;
    
    // Optional UI (overlay) rendering pass.
    virtual void render_ui() {}

    // The resize method will be invoked when the window is resized.
    virtual void resize(uint32_t width, uint32_t height);

    // Adds a subsystem to the application.
    template <typename T, typename... Args>
    T* add_subsystem(Args&&... args);

    // Removes a subsystem from the application.
    template <typename T>
    void remove_subsystem();

    // Gets a subsystem from the application.
    template <typename T>
    T* get_subsystem() const;
    
    // Gets the window handle.
    SDL_Window* get_window() const
    {
        return window;
    }

    // Gets the root directory.
    fs::path get_root_dir() const
    {
        return root_dir;
    }

    // Gets the resources directory.
    fs::path get_resources_dir() const
    {
        return resources_dir;
    }
    
    // Gets the shaders directory.
    fs::path get_shaders_dir() const
    {
        return shaders_dir;
    }

  protected:
    fs::path root_dir;
    fs::path resources_dir;
    fs::path shaders_dir;
    uint32_t display_width = 1920;
    uint32_t display_height = 1080;
    SDL_Window* window = nullptr;
    std::unordered_map<std::type_index, AppSubsystem*> subsystems;
    
    friend int run_application(BaseApp& app, int argc, char* argv[]);
};

template <typename T, typename... Args>
T* BaseApp::add_subsystem(Args&&... args)
{
    auto* subsystem = new T(std::forward<Args>(args)...);
    subsystems[typeid(T)] = subsystem;
    return subsystem;
}

template <typename T>
void BaseApp::remove_subsystem()
{
    if (auto it = subsystems.find(typeid(T)); it != subsystems.end())
    {
        delete it->second;
        subsystems.erase(it);
    }
}

template <typename T>
T* BaseApp::get_subsystem() const
{
    if (auto it = subsystems.find(typeid(T)); it != subsystems.end())
    {
        return dynamic_cast<T*>(const_cast<AppSubsystem*>(it->second));
    }
    return nullptr;
}

int run_application(BaseApp& app, int argc, char* argv[]);
} // namespace ash

// This macro creates the application entry point.
#define ASH_CREATE_APPLICATION(app_class)                                                                              \
    int main(int argc, char* argv[])                                                                                   \
    {                                                                                                                  \
        app_class app;                                                                                                 \
        return ash::run_application(app, argc, argv);                                                                  \
    }
