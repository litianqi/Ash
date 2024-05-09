#include "ash.h"
#include "SDL3/SDL_main.h"
#include "stb/stb_image.h"
#include "glm/ext.hpp"
#include "glm/glm.hpp"

const auto CAMERA_REST_LOCATION = vec3(0.0f, 0.0f, -2.0f);

namespace ash
{
class RendererApp : public BaseApp
{
  public:
    FPSCounter fps_counter;

    std::unique_ptr<World> world;
    GameObjectPtr camera;
    int camera_controller_type = 0; // 0: fly camera, 1: orbit camera

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
        load_gltf(get_resources_dir() / "Sponza/glTF/Sponza.gltf", *world);

        //> create camera
        camera = world->create("Camera", CAMERA_REST_LOCATION);
        auto* camera_component = camera->add_component<CameraComponent>();
        camera_component->fov = 45.0f * (glm::pi<float>() / 180.0f);
        camera_component->aspect_ratio = (float)display_width / (float)display_height;
        camera->add_component<FlyCameraControllerComponent>();

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
        if (camera_controller_type == 0)
        {
            if (camera->has_component<OrbitCameraControllerComponent>())
            {
                camera->remove_components<OrbitCameraControllerComponent>();
                camera->add_component<FlyCameraControllerComponent>();
                camera->set_location(CAMERA_REST_LOCATION);
            }
        }
        else
        {
            if (camera->has_component<FlyCameraControllerComponent>())
            {
                camera->remove_components<FlyCameraControllerComponent>();
                auto* obit = camera->add_component<OrbitCameraControllerComponent>();
                obit->distance = 2.f;
            }
        }
        world->update(dt);
    }

    void render() override
    {
        ZoneScoped;

        const auto* camera_component = camera->get_component<CameraComponent>();
        renderer->render(world.get(), camera_component);
    }

    void render_ui() override
    {
        ImGui::Begin("Hello Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("FPS:    %.2f", fps_counter.get_fps());
        ImGui::Separator();
        ImGui::RadioButton("Fly Camera", &camera_controller_type, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Orbit Camera", &camera_controller_type, 1);
        ImGui::End();
    }
};
} // namespace ash

ASH_CREATE_APPLICATION(ash::RendererApp)
