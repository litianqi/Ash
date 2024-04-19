#pragma once

#include <memory>
#include "core/slot_map_ptr.h"

namespace ash
{
class World;
class Component;
class TransformComponent;
class GameObject;

using GameObjectPtr = SlotMapPtr<GameObject>;

class GameObject
{
  public:
    GameObject() = default;
    
    // Update all components in the game object.
    void update(float dt);

    // Set the parent of the game object.
    void set_parent(GameObjectPtr new_parent);
    
    // Get the parent of the game object.
    GameObjectPtr get_parent() const
    {
        return parent;
    }

    // Add a child to the game object.
    void add_child(GameObjectPtr child);

    // Remove a child from the game object.
    void remove_child(GameObjectPtr child);
    
    // Get the children of the game object.
    const std::vector<GameObjectPtr>& get_children() const
    {
        return children;
    }

    // Add a component to the game object.
    template <class T, typename... Args>
    T* add_component(Args&&... args);

    // Remove all components of the given type from the game object.
    template <class T>
    void remove_components();
    
    // Check if the game object has a component of the given type.
    template <class T>
    bool has_component() const;

    // Get the first component of the given type from the game object.
    template <class T>
    T* get_component() const;

    // Get all components of the given type from the game object.
    template <class T>
    std::vector<T*> get_components() const;
    
    // Get the world that the game object belongs to.
    World* get_world() const
    {
        return world;
    }
    
    // Get the name of the game object.
    const std::string& get_name() const
    {
        return name;
    }
    
    // Get the transform component of the game object.
    TransformComponent* get_transform() const;

  private:
    World* world = nullptr;
    std::string name;
    GameObjectPtr self;
    GameObjectPtr parent;
    std::vector<GameObjectPtr> children;
    std::vector<Component*> components;

    void on_destroy();
    void remove_components(const std::type_info& type);

    friend class World;
};

template <class T, typename... Args>
T* GameObject::add_component(Args&&... args)
{
    auto* component = new T(std::forward<Args>(args)...);
    components.push_back(component);
    component->owner = self;
    component->on_create();
    return component;
}

template <class T>
void GameObject::remove_components()
{
    remove_components(typeid(T));
}

template <class T>
bool GameObject::has_component() const
{
    return get_component<T>() != nullptr;
}

template <class T>
T* GameObject::get_component() const
{
    for (auto& component : components)
    {
        if (auto result = dynamic_cast<T*>(component))
        {
            return result;
        }
    }
    return nullptr;
}

template <class T>
std::vector<T*> GameObject::get_components() const
{
    std::vector<T*> result;
    for (auto& component : components)
    {
        if (auto t_component = dynamic_cast<T*>(component))
        {
            result.push_back(t_component);
        }
    }
    return result;
}
} // namespace ash
