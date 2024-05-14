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
    GameObjectPtr sun;
    vec3 sun_euler = vec3(glm::pi<float>() * 0.25f, 0.f, 0.f);

    std::unique_ptr<ForwardRenderer> renderer;
    ShaderType shader_type = ShaderType::SIMPLE_LIT;

    std::vector<GameObjectPtr> gltf_objects;
    std::vector<std::string> gltf_names;
    std::vector<fs::path> gltf_paths;
    int gltf_idx = 0;

    void list_gltf_files()
    {
        for (fs::recursive_directory_iterator i(get_resources_dir()), end; i != end; ++i)
        {
            if (!fs::is_directory(i->path()) && (i->path().extension() == ".gltf" || i->path().extension() == ".glb"))
            {
                auto file_name = i->path().filename().string();
                size_t dot_index = file_name.find_last_of('.'); 
                gltf_names.push_back(file_name.substr(0, dot_index));
                gltf_paths.push_back(i->path());
            }
        }
    }

    void load_gltf(const fs::path& path)
    {
        for (auto& obj : gltf_objects)
        {
            world->destroy(obj);
        }
        gltf_objects.clear();

        auto gltf = ash::load_gltf(path, *world);
        gltf_objects = gltf->game_objects;
    }

    void startup() override
    {
        BaseApp::startup();
        spdlog::info("Hello, Renderer!");

        //> create world & load glTF into world
        world = std::make_unique<World>();
        list_gltf_files();
        auto it = std::find(gltf_names.begin(), gltf_names.end(), "Sponza");
        gltf_idx = it == gltf_names.end() ? 0 : (int)std::distance(gltf_names.begin(), it);
        load_gltf(gltf_paths[gltf_idx]);

        //> create camera
        camera = world->create("Camera", vec3(0.f));
        auto* camera_component = camera->add_component<CameraComponent>();
        camera_component->fov = 45.0f * (glm::pi<float>() / 180.0f);
        camera_component->aspect_ratio = (float)display_width / (float)display_height;
        add_or_update_camera_controller();

        //> create sun
        sun = world->create("Sun", vec3(0.f, 10.f, 0.f));
        auto* light_component = sun->add_component<DirectionalLightComponent>();
        light_component->color = vec3(1.0f);

        //> create renderer
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
        //        sun->set_rotation(quat(vec3(glm::pi<float>() * 0.25f, get_time_since_startup(), 0.f)));
    }

    void add_or_update_camera_controller()
    {
        if (camera_controller_type == CameraControllerType::Fly &&
            !camera->has_component<FlyCameraControllerComponent>())
        {
            camera->remove_components<OrbitCameraControllerComponent>();
            auto* fly = camera->add_component<FlyCameraControllerComponent>();
            // camera->set_location(vec3(0.0f, 0.0f, -2.0f));
            camera->set_location(vec3(-10, 0, 0));
            fly->pitch = glm::radians(-10.f);
            fly->yaw = glm::radians(90.f);
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
        {
            ImGui::Text("FPS:    %.2f", fps_counter.get_fps());
            ImGui::Separator();

            const char* combo_preview_value = gltf_names[gltf_idx].c_str();
            if (ImGui::BeginCombo("glTF", combo_preview_value))
            {
                for (int i = 0; i < gltf_names.size(); i++)
                {
                    const bool is_selected = (gltf_idx == i);
                    if (ImGui::Selectable(gltf_names[i].c_str(), is_selected))
                    {
                        if (gltf_idx != i)
                        {
                            gltf_idx = i;
                            load_gltf(gltf_paths[i]);
                        }
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::RadioButton("Fly Camera", (int*)&camera_controller_type, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Orbit Camera", (int*)&camera_controller_type, 1);
            add_or_update_camera_controller();

            ImGui::RadioButton("Simple Lit", (int*)&shader_type, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Unlit", (int*)&shader_type, 0);
            renderer->set_shader_type(shader_type);

            ImGui::SliderAngle("Sun Pitch", &sun_euler.x, 0.f, 90.f);
            ImGui::SliderAngle("Sun Yaw", &sun_euler.y, -180.f, 180.f);
            sun->set_rotation(quat(sun_euler));
        }
        ImGui::End();
    }

    void render() override
    {
        ZoneScoped;

        const auto* camera_component = camera->get_component<CameraComponent>();
        renderer->render(world.get(), camera_component, sun->get_component<DirectionalLightComponent>());
    }
};
} // namespace ash

ASH_CREATE_APPLICATION(ash::RendererApp)
