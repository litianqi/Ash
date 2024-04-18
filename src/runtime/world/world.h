#pragma once
#include "slot_map.h"
#include "game_object.h"
#include "core/slot_map_ptr.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
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
    
    void load_gltf(fs::path path);
    
    GameObjectPtr create(const std::string& name, const vec3& location, const quat& rotation = quat(1.f, 0.f, 0.f, 0.f),
                         const vec3& scale = vec3(1.0f));

    void destroy(GameObjectPtr ptr);

    void update(float dt);

  private:
    stdext::slot_map<GameObject> game_objects;
    std::unique_ptr<RenderWorld> render_world;
};
} // namespace ash
