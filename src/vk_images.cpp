//
// Created by JoonHo Son on 2026-04-22.
//

#include "vk_images.h"

#include "vk_initializer.h"

void vkUtil::transitionImage(const VkCommandBuffer buffer, const VkImage image,
                             const VkImageLayout currentLayout,
                             const VkImageLayout nextLayout) {
    VkImageMemoryBarrier2 imageBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = 0,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .dstStageMask = 0,
    };

    imageBarrier.pNext = nullptr;

    // VK_PIPELINE_STAGE_2_ALL_COMMAND_BIT
    //  - GPU 파이프라인을 약간 멈추게 하므로 비효율적일 수 있음
    //  - 단, 예제에서는 프레임마다 변환 수행이 많지 않으므로 문제가 되지 않음
    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask =
        VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = nextLayout;

    VkImageAspectFlags aspectFlags =
        (nextLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
            ? VK_IMAGE_ASPECT_DEPTH_BIT
            : VK_IMAGE_ASPECT_COLOR_BIT;

    imageBarrier.subresourceRange = vkInit::imageSubResourceRange(aspectFlags);
    imageBarrier.image = image;
    VkDependencyInfo dependencyInfo{};

    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = nullptr;

    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(buffer, &dependencyInfo);
}

void vkUtil::copyImageToImage(VkCommandBuffer buffer, VkImage source,
                              VkImage destination, VkExtent2D sourceSize,
                              VkExtent2D destinationSize) {
    VkImageBlit2 blitRegion{.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
                            .pNext = nullptr};

    blitRegion.srcOffsets[1].x = sourceSize.width;
    blitRegion.srcOffsets[1].y = sourceSize.height;
    blitRegion.srcOffsets[1].z = 1;
    blitRegion.dstOffsets[1].x = destinationSize.width;
    blitRegion.dstOffsets[1].y = destinationSize.height;
    blitRegion.dstOffsets[1].z = 1;
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;

    VkBlitImageInfo2 blitInfo{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                              .pNext = nullptr};

    blitInfo.dstImage = destination;
    blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blitInfo.srcImage = source;
    blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blitInfo.filter = VK_FILTER_LINEAR;
    blitInfo.regionCount = 1;
    blitInfo.pRegions = &blitRegion;

    //
    // vkCmdCopyImage2() 함수는 속도는 빠르지만 두 이미지의 해상도가 반드시
    // 같아야 하는 등의 제약이 있음
    //

    vkCmdBlitImage2(buffer, &blitInfo);
}
