//
// Created by JoonHo Son on 2026-04-22.
//

#ifndef VULKAN_GUIDE_VK_IMAGES_H
#define VULKAN_GUIDE_VK_IMAGES_H

#include <vulkan/vulkan.h>

namespace vkUtil {
    void transitionImage(VkCommandBuffer buffer, VkImage image, VkImageLayout currentLayout, VkImageLayout nextLayout);

    void copyImageToImage(VkCommandBuffer buffer, VkImage source, VkImage destination, VkExtent2D sourceSize,
                          VkExtent2D destinationSize);
} // namespace vkUtil

#endif // VULKAN_GUIDE_VK_IMAGES_H
