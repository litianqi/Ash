add_library(Ash
        ash.h
        app/app.cpp
        app/app.h
        app/app_subsystem.h
        core/file_utils.h
        core/file_utils.cpp
        core/fps_counter.h
        core/slot_map_ptr.h
        core/math.h
        gfx/buffer_pool.cpp
        gfx/buffer_pool.h
        gfx/buffer_ring.cpp
        gfx/buffer_ring.h
        gfx/device.cpp
        gfx/device.h
        gfx/imgui.cpp
        gfx/imgui.h
        input/input_manager.cpp
        input/input_manager.h
        input/input_types.h
        renderer/passes/forward_pass.cpp
        renderer/passes/forward_pass.h
        renderer/renderers/forward_renderer.cpp
        renderer/renderers/forward_renderer.h
        renderer/render_list.h
        renderer/render_object.h
        renderer/render_types.h
        renderer/renderer.cpp
        renderer/renderer.h
        resource/resource.h
        resource/mesh_resource.h
        resource/material_resource.h
        resource/texture_resource.h
        resource/gltf_loader.cpp
        resource/gltf_loader.h
        world/components/camera_component.cpp
        world/components/camera_component.h
        world/components/camera_controller_component.cpp
        world/components/camera_controller_component.h
        world/components/light_component.h
        world/components/light_component.cpp
        world/components/mesh_component.cpp
        world/components/mesh_component.h
        world/component.h
        world/game_object.cpp
        world/game_object.h
        world/world.cpp
        world/world.h
        renderer/passes/shadow_pass.cpp
        renderer/passes/shadow_pass.h
)

target_compile_definitions(Ash PUBLIC NOGDI) # disable wingdi.h

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
target_include_directories(Ash PUBLIC ${offsetAllocator_SOURCE_DIR})
target_link_libraries(Ash PUBLIC offsetAllocator)

