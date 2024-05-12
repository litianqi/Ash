#include "world.h"

namespace ash
{
World::World() //: render_world(std::make_unique<RenderWorld>())
{
}

GameObjectPtr World::create(const std::string& name, const vec3& location, const quat& rotation, const vec3& scale)
{
    auto key = game_objects.emplace();
    auto ptr = SlotMapPtr<GameObject>{&game_objects, key};

    auto& game_object = game_objects.at(key);
    game_object.world = this;
    game_object.name = name;
    game_object.self = ptr;

    game_object.set_location(location);
    game_object.set_rotation(rotation);
    game_object.set_scale(scale);

    return ptr;
}

void World::destroy(GameObjectPtr ptr)
{
    if (ptr)
    {
        for (auto& child : ptr->children)
        {
            destroy(child);
        }
        ptr->on_destroy();
        auto key = ptr.get_key();
        game_objects.erase(key);
    }
}

void World::update(float dt)
{
    for (auto& game_object : game_objects)
    {
        game_object.update(dt);
    }
}

World::~World()
{
    for (auto& game_object : game_objects)
    {
        game_object.on_destroy();
    }
    game_objects.clear();
}
} // namespace ash
