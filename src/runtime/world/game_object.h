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
    
    void update(float dt);

    void set_parent(GameObjectPtr new_parent);
    
    GameObjectPtr get_parent() const
    {
        return parent;
    }

    void add_child(GameObjectPtr child);

    void remove_child(GameObjectPtr child);
    
    const std::vector<GameObjectPtr>& get_children() const
    {
        return children;
    }

    template <class T>
    T* add_component();

    template <class T>
    void remove_components();
    
    template <class T>
    bool has_component() const;

    template <class T>
    T* get_component() const;

    template <class T>
    std::vector<T*> get_components() const;
    
    World* get_world() const
    {
        return world;
    }
    
    const std::string& get_name() const
    {
        return name;
    }
    
    TransformComponent* get_transform() const;

  private:
    World* world = nullptr;
    std::string name;
    GameObjectPtr self;
    GameObjectPtr parent;
    std::vector<GameObjectPtr> children;
    std::vector<std::shared_ptr<Component>> components;

    void on_destroy();
    void remove_components(const std::type_info& type);

    friend class World;
};

template <class T>
T* GameObject::add_component()
{
    auto component = std::make_shared<T>();
    components.push_back(component);
    component->owner = self;
    component->on_create();
    return component.get();
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
        if (auto result = dynamic_cast<T*>(component.get()))
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
        if (auto t_component = dynamic_cast<T*>(component.get()))
        {
            result.push_back(t_component);
        }
    }
    return result;
}
} // namespace ash
