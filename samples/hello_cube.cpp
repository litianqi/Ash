#include "app.h"
#include "SDL3/SDL_main.h"

namespace ash
{
class App : public BaseApp
{
public:
    void startup() override
    {
        BaseApp::startup();
        spdlog::info("Hello, Cube!");
    }

    void update(float dt) override
    {
        
    }

    void render() override
    {
        
    }
};
}

ASH_CREATE_APPLICATION(ash::App)

