#include <catch2/catch_test_macros.hpp>
#include "world/world.h"
#include "world/components/transform_component.h"

TEST_CASE("Create and destroy game object", "[World]")
{
    ash::World world;

    auto a = world.create("A", vec3(1, 2, 3));
    REQUIRE(a.is_valid());
    REQUIRE(a->get_world() == &world);
    REQUIRE(a->get_name() == "A");
    REQUIRE(a->get_transform()->get_location() == vec3(1, 2, 3));
    REQUIRE(a->get_transform()->get_rotation() == quat(1, 0, 0, 0));
    REQUIRE(a->get_transform()->get_euler_angles() == vec3(0, 0, 0));
    REQUIRE(a->get_transform()->get_scale() == vec3(1, 1, 1));

    world.destroy(a);
    REQUIRE_FALSE(a.is_valid());
}

TEST_CASE("Attach and detach a game object to another", "[World]")
{
    ash::World world;

    auto a = world.create("A", vec3(1, 2, 3));
    auto b = world.create("A", vec3(1, 2, 3));
    b->set_parent(a);
    REQUIRE(a.is_valid());
    REQUIRE(b->get_parent() == a);
    REQUIRE(a->get_children().size() == 1);
    REQUIRE(a->get_children()[0] == b);
    REQUIRE(a->get_transform()->get_location() == vec3(1, 2, 3));
    REQUIRE(b->get_transform()->get_local_location() == vec3(1, 2, 3));
    REQUIRE(b->get_transform()->get_location() == vec3(2, 4, 6));

    a->remove_child(b);
    REQUIRE_FALSE(b->get_parent().is_valid());
    REQUIRE(a->get_children().empty());
    
    a->add_child(b);
    REQUIRE(b->get_parent() == a);
    REQUIRE(a->get_children().size() == 1);
    REQUIRE(a->get_children()[0] == b);
}

static int32_t test_value = 0;

class TestComponent : public ash::Component
{
  public:
    void on_create() override
    {
        test_value = 1;
    }

    void on_destroy() override
    {
        test_value = 0;
    }

    void update(float dt) override
    {
        test_value += static_cast<int32_t>(dt);
    }
};

TEST_CASE("Custom component", "[World]")
{
    ash::World world;
    
    REQUIRE(test_value == 0);

    auto a = world.create("A", vec3(1, 2, 3));
    auto* comp = a->add_component<TestComponent>();
    REQUIRE(test_value == 1);
    REQUIRE(a->get_components<TestComponent>().size() == 1);
    REQUIRE(comp == a->get_component<TestComponent>());
    
    world.update(2);
    REQUIRE(test_value == 3);
    
    a->remove_components<TestComponent>();
    REQUIRE(test_value == 0);
    
    a->add_component<TestComponent>();
    REQUIRE(test_value == 1);

    world.destroy(a);
    REQUIRE(test_value == 0);
}
