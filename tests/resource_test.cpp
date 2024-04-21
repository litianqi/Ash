#include <catch2/catch_test_macros.hpp>
#include "ash.h"

fs::path resources_dir()
{
    fs::path dir = fs::current_path();
    const char* resources_dir_name = "resources";
    while (dir != fs::current_path().root_path() && !exists(dir / fs::path(resources_dir_name)))
    {
        dir = dir.parent_path();
    }
    return dir / fs::path(resources_dir_name);
}

class TestApp : public ash::BaseApp
{
  public:
    TestApp() = default;
    
    void update(float dt) override
    {
    }
    
    void render() override
    {
    }
};

TEST_CASE("BoxTextured", "[Resource]")
{
    auto path = resources_dir() / "BoxTextured/glTF-Binary/BoxTextured.glb";
    TestApp app;
    app.startup();
    
    ash::ResourceWeakPtr<ash::MeshResource> mesh;
    {
        auto world = std::make_unique<ash::World>();
        auto model = ash::load_gltf(path, *world);

        REQUIRE(model->top_game_objects.size() == 1);
        REQUIRE(model->game_objects.size() == 2);
        REQUIRE(model->meshes.size() == 1);
        REQUIRE(model->materials.size() == 1);
        REQUIRE(model->textures.size() == 1);
        REQUIRE(model->samplers.size() == 1);

        auto mesh_object = model->game_objects[1];
        REQUIRE(mesh_object->has_component<ash::MeshComponent>());

        auto* mesh_component = mesh_object->get_component<ash::MeshComponent>();
        REQUIRE(mesh_component->mesh == model->meshes[0]);

        auto mesh = model->meshes[0];
        REQUIRE(mesh->sub_meshes.size() == 1);
        REQUIRE(mesh->sub_meshes[0].index_offset == 0);
        REQUIRE(mesh->sub_meshes[0].index_count == 36);
        REQUIRE(mesh->sub_meshes[0].material == model->materials[0]);

        auto material = model->materials[0];
        REQUIRE(material->base_color_texture == model->textures[0]);
    }

    app.cleanup();
}

#if ASH_TEST_RESOURCE_MANAGER
class CustomResource;
using CustomResourcePtr = ash::ResourcePtr<CustomResource>;

class CustomResource : public ash::Resource
{
  public:
    CustomResource() = default;

    std::string content;
    bool loaded = false;

    void post_load() override
    {
        loaded = true;
    }

    static ash::LoadedResource<Resource> load(const fs::path& path)
    {
        auto file_content = ash::read_text_file(path);
        if (std::holds_alternative<ash::FileError>(file_content))
        {
            return ash::ResourceLoadError::FILE_NOT_FOUND;
        }
        auto resource = ash::create_resource<CustomResource>();
        resource->content = std::get<std::string>(file_content);
        return resource;
    }
};

TEST_CASE("Sync loading", "[Resource]")
{
    auto& resource_manager = ash::ResourceManager::get();
    resource_manager.register_load_function<CustomResource>(CustomResource::load);
    auto path = resources_dir() / "test/test_file.txt";

    auto a = resource_manager.load<CustomResource>(path);
    REQUIRE(std::holds_alternative<CustomResourcePtr>(a));
    auto a_res = std::get<CustomResourcePtr>(a);
    REQUIRE(a_res->path == path);
    REQUIRE(a_res->loaded);
    REQUIRE(a_res->content == "Hello, Testing!");

    auto a2 = resource_manager.load<CustomResource>(path);
    REQUIRE(std::holds_alternative<CustomResourcePtr>(a2));
    auto a2_res = std::get<CustomResourcePtr>(a2);
    REQUIRE(a2_res == a_res);

    auto a3 = resource_manager.get<CustomResource>(path);
    REQUIRE(a3 == a_res);

    resource_manager.unregister_resource(path);
    auto a4 = resource_manager.get<CustomResource>(path);
    REQUIRE(a4 == nullptr);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
TEST_CASE("Async loading", "[Resource]")
{
    auto& resource_manager = ash::ResourceManager::get();
    resource_manager.register_load_function<CustomResource>(CustomResource::load);

    std::mutex loaded_resources_mutex;
    std::array<CustomResourcePtr, 5> loaded_resources;

    for (int i = 0; i < 5; ++i)
    {
        auto filename = std::format("test/test_file_{}.txt", i);
        auto path = resources_dir() / filename;
        resource_manager.load_async<CustomResource>(path,
                                                    [i, path, &loaded_resources_mutex, &loaded_resources](auto result) {
                                                        REQUIRE(std::holds_alternative<CustomResourcePtr>(result));
                                                        auto resource = std::get<CustomResourcePtr>(result);
                                                        REQUIRE(resource->path == path);
                                                        REQUIRE(resource->loaded);
                                                        REQUIRE(resource->content == "Hello, Testing!");
                                                        std::lock_guard lock(loaded_resources_mutex);
                                                        loaded_resources[i] = resource;
                                                    });
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
    while (loaded_resources.size() < 5)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
#pragma clang diagnostic pop

    for (int i = 0; i < 5; ++i)
    {
        auto filename = std::format("test/test_file_{}.txt", i);
        auto path = resources_dir() / filename;
        auto resource = resource_manager.get<CustomResource>(path);
        REQUIRE(resource);
        REQUIRE(resource == loaded_resources[i]);

        CustomResourcePtr resource2;
        resource_manager.load_async<CustomResource>(
            path, [path, &resource2](auto result) { resource2 = std::get<CustomResourcePtr>(result); });
        REQUIRE(resource2);
        REQUIRE(resource == resource2);
    }
}
#endif
