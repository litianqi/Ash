#include <catch2/catch_test_macros.hpp>
#include "ash.h"

class TestSubsystem : public ash::AppSubsystem
{
  public:
    explicit TestSubsystem(int test_value): test_value(test_value)
    {
    }
    int test_value = 0;
};

class TestApp : public ash::BaseApp
{
  public:
    TestApp() = default;
    
    void startup() override
    {
        ash::BaseApp::startup();
        add_subsystem<TestSubsystem>(1);
    }
    
    void cleanup() override
    {
        remove_subsystem<TestSubsystem>();
        ash::BaseApp::cleanup();
    }
    
    void update(float dt) override
    {
    }
    
    void render() override
    {
    }
};

TEST_CASE("Create and destroy app", "[App]")
{
    TestApp app;
    app.startup();
    REQUIRE(app.get_window() != nullptr);
    REQUIRE(app.get_subsystem<ash::InputManager>() != nullptr);
    REQUIRE(app.get_subsystem<ash::Device>() != nullptr);
    REQUIRE(app.get_subsystem<TestSubsystem>() != nullptr);
    REQUIRE(app.get_subsystem<TestSubsystem>()->test_value == 1);
    
    app.cleanup();
    REQUIRE(app.get_window() == nullptr);
    REQUIRE(app.get_subsystem<ash::InputManager>() == nullptr);
    REQUIRE(app.get_subsystem<ash::Device>() == nullptr);
    REQUIRE(app.get_subsystem<TestSubsystem>() == nullptr);
}
