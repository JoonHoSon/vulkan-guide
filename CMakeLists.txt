cmake_minimum_required(VERSION 4.2)
project(vulkan_guide)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    src/main.cpp
    src/vk_engine.h
    src/vk_engine.cpp
    src/vk_types.h
    src/vk_initializer.h
    src/vk_initializer.cpp
    src/vk_images.h
    src/vk_images.cpp
    src/vk_descriptors.h
    src/vk_descriptors.cpp
    src/vk_pipelines.h
    src/vk_pipelines.cpp
)

find_package(Vulkan REQUIRED)

message(STATUS "Vulkan_INCLUDE_DIRS : ${Vulkan_INCLUDE_DIRS}")

if (WIN32)
    include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
    set(VULKAN_INC_DIR "C:/VulkanSDK/1.4.341.1/Include")
else ()
    set(VULKAN_INC_DIR "${Vulkan_INCLUDE_DIRS}/vulkan")
endif ()

include_directories(${VULKAN_INC_DIR})

# ---------------------------------------------------------------------------------------------------------------
# SDL
#   - SDL3 3.4.4
#   - SDL_image 3.4.2
# ---------------------------------------------------------------------------------------------------------------
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

# ---------------------------------------------------------------------------------------------------------------
# GLM
# ---------------------------------------------------------------------------------------------------------------
find_package(glm CONFIG REQUIRED)

# ---------------------------------------------------------------------------------------------------------------
# FMT
# ---------------------------------------------------------------------------------------------------------------
find_package(fmt CONFIG REQUIRED)

# ---------------------------------------------------------------------------------------------------------------
# VkBootstrap
# ---------------------------------------------------------------------------------------------------------------
find_package(vk-bootstrap REQUIRED)

# ---------------------------------------------------------------------------------------------------------------
# Vulkan Memory Allocator(VMA)
# ---------------------------------------------------------------------------------------------------------------
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

# ---------------------------------------------------------------------------------------------------------------
# spdlog
# ---------------------------------------------------------------------------------------------------------------
find_package(spdlog CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

# ---------------------------------------------------------------------------------------------------------------
# Compile shaders
# ---------------------------------------------------------------------------------------------------------------
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.comp"
)

if (WIN32)
    # TODO(joonho): 2026-04-29 glslangValidator.exe 위치 추가
elseif (Darwin)
    find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/local/bin)
else ()
    find_program(GLSL_VALIDATOR glslangValidator HINTS $ENV{VULKAN_SDK}/bin)
endif ()

message(STATUS "Found GLSL validator : ${GLSL_VALIDATOR}")

foreach (GLSL ${GLSL_SOURCE_FILES})
    message(STATUS "Building shaders...")
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
    message(STATUS "Target file : ${GLSL}")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
        #COMMENT "Compiling ${FILE_NAME} to ${SPIRV}."
    )

    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
    Shaders
    ALL DEPENDS ${SPIRV_BINARY_FILES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    fmt::fmt
    SDL3::SDL3
    glm::glm
    vk-bootstrap::vk-bootstrap
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    spdlog::spdlog
)

