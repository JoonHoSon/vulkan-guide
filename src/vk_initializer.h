//
// Created by JoonHo Son on 2026-04-20.
//

#ifndef VULKAN_GUIDE_VK_INITIALIZER_H
#define VULKAN_GUIDE_VK_INITIALIZER_H

#include "vk_types.h"

namespace vkInit {
    VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1,
                                                          VkCommandBufferLevel level =
                                                                  VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

    VkFramebufferCreateInfo frameBufferCreateInfo(VkRenderPass render_pass, VkExtent2D extent);

    VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);

    VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

    VkSubmitInfo submitInfo(VkCommandBuffer *buffer);

    VkPresentInfoKHR presentInfo();

    VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass render_pass, VkExtent2D window_extent,
                                              VkFramebuffer framebuffer);
}

#endif //VULKAN_GUIDE_VK_INITIALIZER_H
