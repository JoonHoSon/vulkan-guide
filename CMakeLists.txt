cmake_minimum_required(VERSION 4.2)
project(vulkan_guide)

include(FetchContent)

set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(CMAKE_CXX_STANDARD 20)
set(SOURCES
    src/main.cpp
    src/vk_engine.cpp
    src/vk_types.h
    src/vk_initializer.h
    src/vk_initializer.cpp
)

find_package(Vulkan REQUIRED)

# ----------------------------------------------------------------------------------------------------------------------
# SDL
#   - SDL3 3.4.4
#   - SDL_image 3.4.2
# ----------------------------------------------------------------------------------------------------------------------
SET(SDL3_DIR "${VENDOR_DIR}/sdl3")
SET(SDL3_BIN_DIR "${VENDOR_DIR}/sdl3-bin")

FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG refs/tags/release-3.4.4
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(SDL3)

#message("SDL3_BIN_DIR : ${SDL3_BIN_DIR}")
#
#if (NOT EXISTS "${SDL3_BIN_DIR}" OR NOT IS_DIRECTORY "${SDL3_BIN_DIR}")
#    if (NOT EXISTS "${SDL3_DIR}" OR NOT IS_DIRECTORY "${SDL3_DIR}")
#        message("SDL3 download from github repository...")
#
#        FetchContent_Declare(SDL3
#            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
#            GIT_TAG refs/tags/release-3.4.4
#            GIT_SHALLOW TRUE
#            GIT_PROGRESS TRUE
#            SOURCE_DIR "${SDL3_DIR}"
#            BINARY_DIR "${SDL3_BUILD_DIR}"
#            OVERRIDE_FIND_PACKAGE
#        )
#    else ()
#        message("Build from exists source.")
#
#        FetchContent_Declare(SDL3
#            SOURCE_DIR "${SDL3_DIR}"
#            BINARY_DIR "${SDL3_BIN_DIR}"
#            OVERRIDE_FIND_PACKAGE
#        )
#    endif ()
#
#    FetchContent_MakeAvailable(SDL3)
#else ()
#    find_package(SDL3 PATHS "${SDL3_BIN_DIR}" REQUIRED)
#endif ()

# ----------------------------------------------------------------------------------------------------------------------
# GLM
# ----------------------------------------------------------------------------------------------------------------------
SET(GLM_DIR "${VENDOR_DIR}/glm")
SET(GLM_BIN_DIR "${VENDOR_DIR}/glm-bin")

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG refs/tags/1.0.3
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(glm)

#if (NOT EXISTS "${GLM_BIN_DIR}" OR NOT IS_DIRECTORY "${GLM_BIN_DIR}")
#    if (NOT EXISTS "${GLM_DIR}" OR NOT IS_DIRECTORY "${GLM_DIR}")
#        message("GLM download from github repository...")
#
#        FetchContent_Declare(glm
#            GIT_REPOSITORY https://github.com/g-truc/glm.git
#            GIT_TAG refs/tags/1.0.3
#            GIT_SHALLOW TRUE
#            GIT_PROGRESS TRUE
#            SOURCE_DIR "${GLM_DIR}"
#            BINARY_DIR "${GLM_BIN_DIR}"
#            OVERRIDE_FIND_PACKAGE
#        )
#    else ()
#        message("Build from exists source.")
#
#        FetchContent_Declare(glm
#            SOURCE_DIR "${GLM_DIR}"
#            BINARY_DIR "${GLM_BIN_DIR}"
#            OVERRIDE_FIND_PACKAGE
#        )
#    endif ()
#
#    FetchContent_MakeAvailable(glm)
#else ()
#    #    find_package(glm PATHS "${GLM_BIN_DIR}" REQUIRED)
#    #    get_target_property(GLM_INCLUDE_DIRS glm::glm INTERFACE_INCLUDE_DIR)
#    #    FetchContent_MakeAvailable(glm )
#endif ()

include_directories("${SDL3_DIR}/include" "${GLM_DIR}")

# ----------------------------------------------------------------------------------------------------------------------
# FMT
# ----------------------------------------------------------------------------------------------------------------------
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG refs/tags/12.1.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(fmt)

# ----------------------------------------------------------------------------------------------------------------------
# VkBootstrap
# ----------------------------------------------------------------------------------------------------------------------
FetchContent_Declare(vkbootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG refs/tags/v1.4.349
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(vkbootstrap)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    fmt::fmt-header-only
    SDL3::SDL3
    glm::glm
    vk-bootstrap::vk-bootstrap
    Vulkan::Vulkan
)
