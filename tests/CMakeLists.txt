include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
        GIT_TAG         v3.5.3
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(Catch2)

add_executable(AshTests 
        world_test.cpp
        app_test.cpp)

target_include_directories(HelloCube PRIVATE ${ASH_INCLUDE_DIR})
target_link_libraries(AshTests PRIVATE Ash Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(AshTests)
