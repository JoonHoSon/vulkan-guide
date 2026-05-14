//
// Created by JoonHo Son on 2026-04-20.
//

#ifndef VULKAN_GUIDE_VK_INITIALIZER_H
#define VULKAN_GUIDE_VK_INITIALIZER_H

#include <string>

#include "vk_types.h"
#include "vulkan_core.h"

namespace vkInit {
[[nodiscard]]
VkCommandPoolCreateInfo commandPoolCreateInfo(
    uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

[[nodiscard]]
VkCommandBufferAllocateInfo commandBufferAllocateInfo(
    VkCommandPool pool, uint32_t count = 1,
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

[[nodiscard]]
VkCommandBufferBeginInfo commandBufferBeginInfo(
    VkCommandBufferUsageFlags flags = 0);

[[nodiscard]]
VkFramebufferCreateInfo frameBufferCreateInfo(VkRenderPass render_pass,
                                              VkExtent2D extent);

[[nodiscard]]
VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);

[[nodiscard]]
VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

[[nodiscard]]
VkPresentInfoKHR presentInfo();

[[nodiscard]]
VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass render_pass,
                                          VkExtent2D window_extent,
                                          VkFramebuffer framebuffer);
[[nodiscard]]
VkImageSubresourceRange imageSubResourceRange(VkImageAspectFlags flags);

[[nodiscard]]
VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask,
                                          VkSemaphore semaphore);

[[nodiscard]]
VkCommandBufferSubmitInfo commandSubmitInfo(VkCommandBuffer buffer);

[[nodiscard]]
VkSubmitInfo2 submitInfo(const VkCommandBufferSubmitInfo *commandSubmitInfo,
                         const VkSemaphoreSubmitInfo *signalSemaphoreSubmitInfo,
                         const VkSemaphoreSubmitInfo *waitSemaphoreSubmitInfo);

[[nodiscard]]
VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags flags,
                                  VkExtent3D extent);

[[nodiscard]]
VkImageViewCreateInfo imageViewCreateInfo(VkFormat format, VkImage image,
                                          VkImageAspectFlags flags);

[[nodiscard]]
VkRenderingAttachmentInfo attachmentInfo(VkImageView view, VkClearValue *clear,
                                         VkImageLayout layout);

[[nodiscard]]
VkRenderingInfo renderingInfo(VkExtent2D extent,
                              VkRenderingAttachmentInfo *colorAttachment,
                              VkRenderingAttachmentInfo *depthAttachment);
}  // namespace vkInit

#endif  // VULKAN_GUIDE_VK_INITIALIZER_H
