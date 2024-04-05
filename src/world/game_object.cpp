#include "game_object.h"
#include "components/transform_component.h"
#include "utils/stl_utils.h"

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
            vector_erase_first(parent->children, self);
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
    if (child && !vector_contains(children, child))
    {
        children.push_back(child);
        child->parent = self;
    }
}

void GameObject::remove_child(GameObjectPtr child)
{
    if (child && vector_contains(children, child))
    {
        vector_erase_first(children, child);
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
