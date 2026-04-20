//
// Created by JoonHo Son on 2026-04-20.
//

#ifndef VULKAN_GUIDE_VK_TYPES_H
#define VULKAN_GUIDE_VK_TYPES_H

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
// #include <vk_mem_alloc.h>
#include <fmt/core.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#define VK_CHECK(X)\
    do { \
        VkResult err = x;\
        if (err) { \
            fmt::print("Detected Vulkan error: {}", string_VkResult(err));\
        }\
    } while (0)
#endif //VULKAN_GUIDE_VK_TYPES_H
