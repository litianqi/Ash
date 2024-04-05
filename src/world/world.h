#pragma once
#include "slot_map.h"
#include "game_object.h"
#include "utils/slot_map_ptr.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

using glm::quat;
using glm::vec3;

namespace ash
{
class World
{
  public:
    GameObjectPtr create_game_object(const std::string& name, const vec3& location,
                                     const quat& rotation = quat(1.f, 0.f, 0.f, 0.f), const vec3& scale = vec3(1.0f));

    void destroy_game_object(GameObjectPtr ptr);
    
    void update(float dt);

  private:
    stdext::slot_map<GameObject> game_objects;
};
} // namespace ash
