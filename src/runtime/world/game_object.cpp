#include "game_object.h"
#include "components/transform_component.h"
#include "core/file.h"

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
    }
}

void GameObject::add_child(GameObjectPtr child)
{
    if (child && std::find(children.begin(), children.end(), child) == children.end())
    {
        children.push_back(child);
        child->parent = self;
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
    }
}

TransformComponent* GameObject::get_transform() const
{
    return get_component<TransformComponent>();
}

void GameObject::on_destroy()
{
    for (auto& component : components)
    {
        component->on_destroy();
    }
    components.clear();
}

void GameObject::remove_components(const type_info& type)
{
    for (auto it = components.begin(); it != components.end();)
    {
        if (typeid(**it) == type && typeid(**it) != typeid(TransformComponent)) // Don't allow to remove transform component
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
}
