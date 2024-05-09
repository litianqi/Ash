#include "ash.h"
#include "SDL3/SDL_main.h"
#include "stb/stb_image.h"
#include "glm/ext.hpp"
#include "glm/glm.hpp"

namespace
{
enum class CameraControllerType : int
{
    Fly = 0,
    Orbit = 1,
};
} // namespace

namespace ash
{
class RendererApp : public BaseApp
{
  public:
    FPSCounter fps_counter;

    std::unique_ptr<World> world;
    GameObjectPtr camera;
    CameraControllerType camera_controller_type = CameraControllerType::Fly;

    std::unique_ptr<ForwardRenderer> renderer;

    void startup() override
    {
        BaseApp::startup();
        spdlog::info("Hello, Renderer!");

        //> create world & load glTF into world
        world = std::make_unique<World>();
        // load_gltf(get_resources_dir() / "BoxTextured/glTF-Binary/BoxTextured.glb", *world);
        // load_gltf(get_resources_dir() / "FlightHelmet/glTF/FlightHelmet.gltf", *world);
        // load_gltf(get_resources_dir() / "DamagedHelmet/glTF-Binary/DamagedHelmet.glb", *world);
        // load_gltf(get_resources_dir() / "Bistro_Godot.glb", *world);
        load_gltf(get_resources_dir() / "Sponza/glTF/Sponza.gltf", *world);

        //> create camera
        camera = world->create("Camera", vec3(0.f));
        auto* camera_component = camera->add_component<CameraComponent>();
        camera_component->fov = 45.0f * (glm::pi<float>() / 180.0f);
        camera_component->aspect_ratio = (float)display_width / (float)display_height;
        add_or_update_camera_controller();

        renderer = std::make_unique<ForwardRenderer>(*Device::get(),
                                                     RendererDesc{.width = display_width, .height = display_height});
    }

    void cleanup() override
    {
        renderer = nullptr;
        world = nullptr;
        BaseApp::cleanup();
    }

    void resize(uint32_t width, uint32_t height) override
    {
        BaseApp::resize(width, height);
        renderer->resize(width, height);
    }

    void update(float dt) override
    {
        fps_counter.tick(dt);
        world->update(dt);
    }
    
    void add_or_update_camera_controller()
    {
        if (camera_controller_type == CameraControllerType::Fly &&
            !camera->has_component<FlyCameraControllerComponent>())
        {
            camera->remove_components<OrbitCameraControllerComponent>();
            camera->add_component<FlyCameraControllerComponent>();
            camera->set_location(vec3(0.0f, 0.0f, -2.0f));
        }
        else if (camera_controller_type == CameraControllerType::Orbit &&
                 !camera->has_component<OrbitCameraControllerComponent>())
        {
            camera->remove_components<FlyCameraControllerComponent>();
            auto* obit = camera->add_component<OrbitCameraControllerComponent>();
            obit->distance = 2.f;
        }
    }
    
    void render_ui() override
    {
        ImGui::Begin("Hello Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("FPS:    %.2f", fps_counter.get_fps());
        ImGui::Separator();

        ImGui::RadioButton("Fly Camera", (int*)&camera_controller_type, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Orbit Camera", (int*)&camera_controller_type, 1);
        add_or_update_camera_controller();
        ImGui::End();
    }

    void render() override
    {
        ZoneScoped;

        const auto* camera_component = camera->get_component<CameraComponent>();
        renderer->render(world.get(), camera_component);
    }
};
} // namespace ash

ASH_CREATE_APPLICATION(ash::RendererApp)
