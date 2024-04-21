#pragma once
#include "slot_map.h"
#include "game_object.h"
#include "core/slot_map_ptr.h"
#include "core/math.h"
#include <filesystem>

using glm::quat;
using glm::vec3;
namespace fs = std::filesystem;

namespace ash
{
class World
{
  public:
    World();
    ~World();
    
//    void load_gltf(fs::path path);
    
    // Create a new game object with the given name, location, rotation, and scale.
    GameObjectPtr create(const std::string& name, const vec3& location, const quat& rotation = quat(1.f, 0.f, 0.f, 0.f),
                         const vec3& scale = vec3(1.0f));

    // Destroy the game object with the given pointer.
    void destroy(GameObjectPtr ptr);

    // Update all game objects in the world.
    void update(float dt);

  private:
    stdext::slot_map<GameObject> game_objects;
//    std::unique_ptr<RenderWorld> render_world;
};
} // namespace ash
