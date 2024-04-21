add_library(Ash
        ash.h
        app/app.cpp
        app/app.h
        app/app_subsystem.cpp
        app/app_subsystem.h
        core/fps_counter.h
        core/slot_map_ptr.h
        core/file.h
        core/math.h
        gfx/device.cpp
        gfx/device.h
        input/input_manager.cpp
        input/input_manager.h
        world/world.cpp
        world/world.h
        world/game_object.cpp
        world/game_object.h
        world/components/camera_component.cpp
        world/components/camera_component.h
        world/components/camera_controller_component.cpp
        world/components/camera_controller_component.h
        world/component.h
)

target_include_directories(Ash PUBLIC ${ASH_INCLUDE_DIR})
target_include_directories(Ash PUBLIC ${VULKAN_PATH}/Include)
target_link_directories(Ash PUBLIC ${VULKAN_PATH}/Bin PUBLIC ${VULKAN_PATH}/Lib;)
target_link_libraries(Ash PUBLIC glm::glm-header-only)
target_link_libraries(Ash PUBLIC SDL3::SDL3)
target_include_directories(Ash PUBLIC ${stb_SOURCE_DIR})
target_link_libraries(Ash PUBLIC LVKLibrary)
target_link_libraries(Ash PUBLIC simdjson::simdjson)
target_link_libraries(Ash PUBLIC fastgltf::fastgltf)
target_link_libraries(Ash PUBLIC spdlog::spdlog_header_only)
target_link_libraries(Ash PUBLIC sg14)
target_link_libraries(Ash PUBLIC Taskflow)
