#pragma once

#include <cassert>
#include "game_object.h"

namespace ash
{
class Component
{
  public:
    virtual ~Component() = default;

    // Called when the component is created.
    virtual void on_create()
    {
    }

    // Called when the component is destroyed.
    virtual void on_destroy()
    {
    }

    // Called when the component is updated.
    virtual void update(float dt)
    {
    }
    
    // Get the owner of the component.
    GameObjectPtr get_owner() const
    {
        return owner;
    }
    
    // Get the world the component is in.
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
