#pragma once

#include <cassert>
#include "game_object.h"

namespace ash
{
class Component
{
  public:
    virtual ~Component() = default;

    virtual void on_create()
    {
    }

    virtual void on_destroy()
    {
    }

    virtual void update(float dt)
    {
    }

    [[nodiscard]]
    GameObjectPtr get_owner() const
    {
        return owner;
    }
    
    [[nodiscard]]
    World* get_world() const
    {
        assert(owner);
        return owner->get_world();
    }

  protected:
    GameObjectPtr owner;
    friend GameObject;
};
} // namespace ash
