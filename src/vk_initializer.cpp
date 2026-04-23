//
// Created by JoonHo Son on 2026-04-20.
//

#include "vk_initializer.h"

VkCommandPoolCreateInfo vkInit::commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo info;

    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.queueFamilyIndex = queueFamilyIndex;
    info.flags = flags;

    return info;
}

VkCommandBufferAllocateInfo vkInit::commandBufferAllocateInfo(const VkCommandPool pool,
                                                              const uint32_t count,
                                                              const VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo info;

    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.commandPool = pool;
    info.commandBufferCount = count;
    info.level = level;

    return info;
}

VkCommandBufferBeginInfo vkInit::commandBufferBeginInfo(VkCommandBufferUsageFlags const flags) {
    VkCommandBufferBeginInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.pInheritanceInfo = nullptr;
    info.flags = flags;

    return info;
}

VkFenceCreateInfo vkInit::fenceCreateInfo(VkFenceCreateFlags flags) {
    VkFenceCreateInfo info;

    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;

    return info;
}

VkSemaphoreCreateInfo vkInit::semaphoreCreateInfo(VkSemaphoreCreateFlags flags) {
    VkSemaphoreCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;

    return info;
}

VkImageSubresourceRange vkInit::imageSubResourceRange(const VkImageAspectFlags flags) {
    VkImageSubresourceRange subImage{};

    subImage.aspectMask = flags;
    subImage.baseMipLevel = 0;
    subImage.levelCount = VK_REMAINING_MIP_LEVELS;
    subImage.baseArrayLayer = 0;
    subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

    return subImage;
}

VkSemaphoreSubmitInfo vkInit::semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore) {
    VkSemaphoreSubmitInfo info{};

    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    info.pNext = nullptr;
    info.semaphore = semaphore;
    info.stageMask = stageMask;
    info.deviceIndex = 0;
    info.value = 1;

    return info;
}

VkCommandBufferSubmitInfo vkInit::commandSubmitInfo(VkCommandBuffer buffer) {
    VkCommandBufferSubmitInfo info{};

    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    info.pNext = nullptr;
    info.commandBuffer = buffer;
    info.deviceMask = 0;

    return info;
}

VkSubmitInfo2 vkInit::submitInfo(const VkCommandBufferSubmitInfo *commandSubmitInfo,
                                 const VkSemaphoreSubmitInfo *signalSemaphoreSubmitInfo,
                                 const VkSemaphoreSubmitInfo *waitSemaphoreSubmitInfo) {
    VkSubmitInfo2 info{};

    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    info.pNext = nullptr;
    info.waitSemaphoreInfoCount = waitSemaphoreSubmitInfo == nullptr? 0: 1;
    info.pWaitSemaphoreInfos = waitSemaphoreSubmitInfo;
    info.signalSemaphoreInfoCount = signalSemaphoreSubmitInfo == nullptr? 0: 1;
    info.pSignalSemaphoreInfos = signalSemaphoreSubmitInfo;
    info.commandBufferInfoCount = 1;
    info.pCommandBufferInfos = commandSubmitInfo;

    return info;
}
