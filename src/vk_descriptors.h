//
// Created by JoonHo Son on 2026-04-29.
//

#ifndef VULKAN_GUIDE_VK_DESCRIPTORS_H
#define VULKAN_GUIDE_VK_DESCRIPTORS_H
#include <span>
#include <vector>
#include <vulkan_core.h>

struct DescriptorLayoutBuilder {
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    void addBinding(uint32_t binding, VkDescriptorType type);
    void clear();
    VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags stageFlags, void* pNext = nullptr,
                                VkDescriptorSetLayoutCreateFlags createFlags = 0);
};

struct DescriptorAllocator {
    struct PoolSizeRatio {
        VkDescriptorType type;
        float ratio;
    };

    VkDescriptorPool pool;

    void initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
    void clearDescriptors(VkDevice device);
    void destroyPool(VkDevice device);
    VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

#endif // VULKAN_GUIDE_VK_DESCRIPTORS_H
