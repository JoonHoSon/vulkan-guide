//
// Created by JoonHo Son on 2026-04-20.
//

#ifndef VULKAN_GUIDE_VK_TYPES_H
#define VULKAN_GUIDE_VK_TYPES_H

#if !defined(NDEBUG) || defined(_DEBUG)
#define DEBUG_BUILD
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

// fmt::print("Detected Vulkan error: {}\n", string_VkResult(err));
#define VK_CHECK(x)                                                          \
    do {                                                                     \
        VkResult err = x;                                                    \
        if (err) {                                                           \
            SPDLOG_ERROR("Detected Vulkan error: {}", string_VkResult(err)); \
        }                                                                    \
    } while (0)

struct AllocatedImage {
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
    VkExtent3D imageExtent;
    VkFormat imageFormat;
};

#endif  // VULKAN_GUIDE_VK_TYPES_H
