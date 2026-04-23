//
// Created by JoonHo Son on 2026-04-22.
//

#include "vk_images.h"
#include "vk_initializer.h"

void vkUtil::transitionImage(const VkCommandBuffer buffer, const VkImage image, const VkImageLayout currentLayout,
                             const VkImageLayout nextLayout) {
    VkImageMemoryBarrier2 imageBarrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};

    imageBarrier.pNext = nullptr;

    // VK_PIPELINE_STAGE_2_ALL_COMMAND_BIT
    //  - GPU 파이프라인을 약간 멈추게 하므로 비효율적일 수 있음
    //  - 단, 예제에서는 프레임마다 변환 수행이 많지 않으므로 문제가 되지 않음
    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = nextLayout;

    VkImageAspectFlags aspectFlags = (nextLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                                         ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    imageBarrier.subresourceRange = vkInit::imageSubResourceRange(aspectFlags);
    imageBarrier.image = image;
    VkDependencyInfo dependencyInfo{};

    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = nullptr;

    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(buffer, &dependencyInfo);
}
