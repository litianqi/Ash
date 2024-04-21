#pragma once

#include <memory>
#include "core/slot_map_ptr.h"
#include "core/math.h"

namespace ash
{
class World;
class Component;
class GameObject;

using GameObjectPtr = SlotMapPtr<GameObject>;

class GameObject
{
  public:
    GameObject() = default;
    
    // Update all components in the game object.
    void update(float dt);
    
    ////////////////////////////////////////////////////////////////////////////
    // Components
    ////////////////////////////////////////////////////////////////////////////
    
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
    
    ////////////////////////////////////////////////////////////////////////////
    // Hierarchy
    ////////////////////////////////////////////////////////////////////////////

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
    
    ////////////////////////////////////////////////////////////////////////////
    // Transform
    ////////////////////////////////////////////////////////////////////////////
    
    /////////////////////////// Location ////////////////////////////
    
    // Get the location of the transform.
    vec3 get_location() const;

    // Get the local location of the transform.
    vec3 get_local_location() const
    {
        return local_location;
    }
    
    // Set the location of the transform.
    void set_location(const vec3& value);
    
    // Set the local location of the transform.
    void set_local_location(const vec3& value);
    
    /////////////////////////// Rotation ////////////////////////////
    
    // Get the rotation of the transform.
    quat get_rotation() const;
    
    // Get the local rotation of the transform.
    quat get_local_rotation() const
    {
        return local_rotation;
    }
    
    // Set the rotation of the transform.
    void set_rotation(const quat& value);
    
    // Set the local rotation of the transform.
    void set_local_rotation(const quat& value);
    
    /////////////////////////// Scale ////////////////////////////
    
    // Get the scale of the transform.
    vec3 get_scale() const;
    
    // Get the local scale of the transform.
    vec3 get_local_scale() const
    {
        return local_scale;
    }
    
    // Set the scale of the transform.
    void set_scale(const vec3& value);
    
    // Set the local scale of the transform.
    void set_local_scale(const vec3& value);
    
    /////////////////////////// Direction ////////////////////////////
    
    // Get the forward vector of the transform.
    vec3 get_forward() const;
    
    // Get the right vector of the transform.
    vec3 get_right() const;
    
    // Get the up vector of the transform.
    vec3 get_up() const;
    
    /////////////////////////// Matrix ////////////////////////////
    
    // Get the local to world matrix of the transform.
    const mat4& get_matrix() const
    {
        return matrix;
    }
    
    // Get the local to parent matrix of the transform.
    const mat4& get_local_matrix() const
    {
        return local_matrix;
    }

    // Set the local to world matrix of the transform.
    void set_matrix(const mat4& value);
    
    // Set the local to parent matrix of the transform.
    void set_local_matrix(const mat4& value);
    
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

  private:
    World* world = nullptr;
    std::string name;
    std::vector<Component*> components;
    GameObjectPtr self;
    GameObjectPtr parent;
    std::vector<GameObjectPtr> children;
    vec3 local_location = vec3(0.0, 0.0, 0.0);
    quat local_rotation = quat(1.0, 0.0, 0.0, 0.0);
    vec3 local_scale = vec3(1.0, 1.0, 1.0);
    glm::mat4 matrix = glm::mat4(1.0);
    glm::mat4 local_matrix = glm::mat4(1.0);

    void on_destroy();
    void remove_components(const std::type_info& type);
    void update_children_matrix();

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
