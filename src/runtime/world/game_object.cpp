#include "game_object.h"
#include "component.h"
#include "core/math.h"

namespace ash
{
void GameObject::update(float dt)
{
    for (auto& component : components)
    {
        component->update(dt);
    }
}

void GameObject::set_parent(GameObjectPtr new_parent)
{
    if (parent != new_parent)
    {
        if (parent)
        {
            auto it = std::find(parent->children.begin(), parent->children.end(), self);
            if (it != parent->children.end())
            {
                children.erase(it);
            }
        }
        parent = new_parent;
        if (new_parent)
        {
            new_parent->children.push_back(self);
        }
        matrix = parent ? parent->matrix * local_matrix : local_matrix;
        update_children_matrix();
    }
}

void GameObject::add_child(GameObjectPtr child)
{
    if (child && std::find(children.begin(), children.end(), child) == children.end())
    {
        children.push_back(child);
        child->parent = self;
        child->matrix = matrix * child->local_matrix;
        child->update_children_matrix();
    }
}

void GameObject::remove_child(GameObjectPtr child)
{
    if (child && std::find(children.begin(), children.end(), child) != children.end())
    {
        auto it = std::find(children.begin(), children.end(), child);
        if (it != children.end())
        {
            children.erase(it);
        }
        child->parent = {};
        child->matrix = child->local_matrix;
        child->update_children_matrix();
    }
}

void GameObject::on_destroy()
{
    for (auto& component : components)
    {
        component->on_destroy();
        delete component;
    }
    components.clear();
}

void GameObject::remove_components(const std::type_info& type)
{
    for (auto it = components.begin(); it != components.end();)
    {
        if (typeid(**it) == type)
        {
            (*it)->on_destroy();
            it = components.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

vec3 GameObject::get_location() const
{
    return mat4_decompose_translation(matrix);
}

void GameObject::set_location(const vec3& value)
{
    if (parent)
    {
        const auto new_local_location = glm::inverse(parent->matrix) * vec4(value, 1.0f);
        set_local_location(new_local_location);
    }
    else
    {
        set_local_location(value);
    }
}

void GameObject::set_local_location(const vec3& value)
{
    if (local_location != value) 
    {
        local_location = value;
        local_matrix = mat4_compose(local_scale, local_rotation, local_location);
        matrix = parent ? parent->matrix * local_matrix : local_matrix;
        update_children_matrix();
    }
}

quat GameObject::get_rotation() const
{
    vec3 scale;
    quat rotation;
    mat4_decompose_scale_rotation(matrix, scale, rotation);
    return rotation;
}

void GameObject::set_rotation(const quat& value)
{
    if (parent)
    {
        const auto new_local_rotation = glm::inverse(parent->get_rotation()) * value;
        set_local_rotation(new_local_rotation);
    }
    else
    {
        set_local_rotation(value);
    }
}

void GameObject::set_local_rotation(const quat& value)
{
    if (local_rotation != value) 
    {
        local_rotation = value;
        local_matrix = mat4_compose(local_scale, local_rotation, local_location);
        matrix = parent ? parent->matrix * local_matrix : local_matrix;
        update_children_matrix();
    }
}

vec3 GameObject::get_scale() const
{
    return mat4_decompose_scale(matrix);
}

void GameObject::set_scale(const vec3& value)
{
    if (parent)
    {
        const auto new_local_scale = vec3_reciprocal(parent->get_scale()) * value;
        set_local_scale(new_local_scale);
    }
    else
    {
        set_local_scale(value);
    }
}
    
void GameObject::set_local_scale(const vec3& value)
{
    if (local_scale != value) 
    {
        local_scale = value;
        local_matrix = mat4_compose(local_scale, local_rotation, local_location);
        matrix = parent ? parent->matrix * local_matrix : local_matrix;
        update_children_matrix();
    }
}

vec3 GameObject::get_forward() const
{
    return mat3_cast(local_rotation) * FORWARD_VECTOR;
}
    
vec3 GameObject::get_right() const
{
    return mat3_cast(local_rotation) * RIGHT_VECTOR;
}
    
vec3 GameObject::get_up() const
{
    return mat3_cast(local_rotation) * UP_VECTOR;
}

void GameObject::set_matrix(const mat4& value)
{
    if (parent)
    {
        matrix = value;
        local_matrix = glm::inverse(parent->matrix) * value;
    }
    else
    {
        matrix = value;
        local_matrix = value;
    }
    update_children_matrix();
}
    
void GameObject::set_local_matrix(const mat4& value)
{
    local_matrix = value;
    matrix = parent ? parent->matrix * local_matrix : local_matrix;
    update_children_matrix();
}

void GameObject::update_children_matrix()
{
    for (auto& child : children)
    {
        child->matrix = matrix * child->local_matrix;
        child->update_children_matrix();
    }
}
}
