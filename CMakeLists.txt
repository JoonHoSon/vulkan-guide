cmake_minimum_required(VERSION 4.2)
project(vulkan_guide)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    src/main.cpp
    src/vk_engine.cpp
    src/vk_types.h
    src/vk_initializer.h
    src/vk_initializer.cpp
)

find_package(Vulkan REQUIRED)

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
find_package(glm REQUIRED)

# ---------------------------------------------------------------------------------------------------------------
# FMT
# ---------------------------------------------------------------------------------------------------------------
find_package(fmt REQUIRED)

# ---------------------------------------------------------------------------------------------------------------
# VkBootstrap
# ---------------------------------------------------------------------------------------------------------------
find_package(vk-bootstrap REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    fmt::fmt
    SDL3::SDL3
    glm::glm
    vk-bootstrap::vk-bootstrap
    Vulkan::Vulkan
)
