//
// Created by JoonHo Son on 2026-04-29.
//

#ifndef VULKAN_GUIDE_VK_PIPELINES_H
#define VULKAN_GUIDE_VK_PIPELINES_H
#include <vulkan_core.h>

namespace vkUtil {
bool loadShaderModule(const char *filePath, VkDevice device,
                      VkShaderModule *shaderModule);
}

#endif  // VULKAN_GUIDE_VK_PIPELINES_H
