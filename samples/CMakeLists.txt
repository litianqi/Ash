add_executable(HelloWindow hello_window.cpp)
target_link_libraries(HelloWindow PRIVATE SDL3::SDL3)
target_link_libraries(HelloWindow PRIVATE spdlog::spdlog_header_only)

add_executable(HelloTriangle hello_triangle.cpp)
target_link_libraries(HelloTriangle PRIVATE SDL3::SDL3)
target_link_libraries(HelloTriangle PRIVATE spdlog::spdlog_header_only)
target_link_libraries(HelloTriangle PRIVATE LVKLibrary)

add_executable(EmptyApp empty_app.cpp)
target_link_libraries(EmptyApp PRIVATE Ash)
target_include_directories(EmptyApp PRIVATE ${ASH_INCLUDE_DIR})

add_executable(HelloCube hello_cube.cpp)
target_link_libraries(HelloCube PRIVATE Ash)
target_include_directories(HelloCube PRIVATE ${ASH_INCLUDE_DIR})

add_executable(HelloCamera hello_camera.cpp)
target_link_libraries(HelloCamera PRIVATE Ash)
target_include_directories(HelloCamera PRIVATE ${ASH_INCLUDE_DIR})

add_executable(HelloGltf hello_gltf.cpp)
target_link_libraries(HelloGltf PRIVATE Ash)
target_include_directories(HelloGltf PRIVATE ${ASH_INCLUDE_DIR})

add_executable(HelloRenderer hello_renderer.cpp)
target_link_libraries(HelloRenderer PRIVATE Ash)
target_include_directories(HelloRenderer PRIVATE ${ASH_INCLUDE_DIR})
