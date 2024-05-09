cmake_minimum_required(VERSION 3.27)
project(Ash)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(ASH_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ASH_INCLUDE_DIR "${ASH_ROOT_DIR}/src/runtime")
message(STATUS "ASH_ROOT_DIR = " ${ASH_ROOT_DIR})
message(STATUS "ASH_INCLUDE_DIR = " ${ASH_INCLUDE_DIR})

option(ASH_BUILD_SAMPLES "Enables builds of samples" ON)
option(ASH_BUILD_TESTS "Enables builds of tests" ON)
message(STATUS "ASH_BUILD_SAMPLES: " ${ASH_BUILD_SAMPLES})
message(STATUS "ASH_BUILD_TESTS: " ${ASH_BUILD_TESTS})


# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

message(STATUS "Attempting auto locate Vulkan using CMake......")
cmake_path(SET VULKAN_PATH $ENV{VULKAN_SDK})
if (${VULKAN_PATH} STREQUAL "" OR NOT EXISTS "${VULKAN_PATH}")
    message(FATAL_ERROR "Error: Unable to locate Vulkan SDK, please check if the Vulkan SDK is installed correctly.")
endif()
message(STATUS "Found Vulkan: ${VULKAN_PATH}")

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
if(NOT EXISTS "${GLSL_VALIDATOR}")
    message(FATAL_ERROR "Error: Unable to find glslangValidator, please check if the Vulkan SDK is installed correctly.")
endif()
message(STATUS "Found glslangValidator: ${GLSL_VALIDATOR}")

include(FetchContent)

set(GLM_ENABLE_CXX_20 ON)
FetchContent_Declare(
        glm
        GIT_REPOSITORY  https://github.com/icaven/glm.git
        GIT_TAG         dca38025fba63bb9284023e6de55f756b9e37cec
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(glm)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
FetchContent_Declare(
        SDL
        GIT_REPOSITORY  https://github.com/libsdl-org/SDL.git
        GIT_TAG         prerelease-3.1.1
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(SDL)

FetchContent_Declare(
        stb
        GIT_REPOSITORY  https://github.com/nothings/stb.git
        GIT_TAG         ae721c50eaf761660b4f90cc590453cdb0c2acd0
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(stb)

set(LVK_DEPLOY_DEPS ON)
set(LVK_WITH_GLFW OFF)
set(LVK_WITH_SAMPLES OFF)
set(TRACY_DELAYED_INIT ON)
FetchContent_Declare(
        lvk
        GIT_REPOSITORY  https://github.com/litianqi/lightweightvk.git
        GIT_TAG         2c1ed20f626cec3a1e13abe21a9df529a8fe1b44
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(lvk)

FetchContent_Declare(
        simdjson
        GIT_REPOSITORY  https://github.com/simdjson/simdjson.git
        GIT_TAG         v3.8.0
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(simdjson)

set(FASTGLTF_COMPILE_AS_CPP20 ON)
FetchContent_Declare(
        fastgltf
        GIT_REPOSITORY  https://github.com/spnda/fastgltf.git
        GIT_TAG         v0.7.1
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(fastgltf)

set(SPDLOG_USE_STD_FORMAT ON)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY  https://github.com/gabime/spdlog.git
        GIT_TAG         v1.13.0
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
        sg14
        GIT_REPOSITORY  https://github.com/WG21-SG14/SG14.git
        GIT_TAG         c92614381100bd23f3f0de7882aa455797255c03
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(sg14)

set(TF_BUILD_TESTS OFF)
set(TF_BUILD_EXAMPLES OFF)
FetchContent_Declare(
        taskflow
        GIT_REPOSITORY  https://github.com/taskflow/taskflow.git
        GIT_TAG         1b6cf17bb37741103a67cb3698066b6b80def2e0
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(taskflow)

function(setup_target_libs)
endfunction()
FetchContent_Declare(
        offsetAllocator
        GIT_REPOSITORY  https://github.com/litianqi/OffsetAllocator.git
        GIT_TAG         97b8fb8f8b8be975efc98410ee58439f2a154254
        GIT_PROGRESS    TRUE
)
FetchContent_MakeAvailable(offsetAllocator)

# -----------------------------------------------------------------------------
# Ash Library
# -----------------------------------------------------------------------------

add_subdirectory(src/runtime)

# -----------------------------------------------------------------------------
# Ash Editor
# -----------------------------------------------------------------------------

add_subdirectory(src/editor)

# -----------------------------------------------------------------------------
# Samples
# -----------------------------------------------------------------------------

if(ASH_BUILD_SAMPLES)
    add_subdirectory(samples)
endif(ASH_BUILD_SAMPLES)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------

if(ASH_BUILD_TESTS)
    add_subdirectory(tests)
endif(ASH_BUILD_TESTS)

