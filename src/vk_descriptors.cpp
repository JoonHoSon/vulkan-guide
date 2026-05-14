//
// Created by JoonHo Son on 2026-04-29.
//
#include "vk_descriptors.h"

#include "vk_types.h"

// ----------------------------------------------------------------------------------------------
// DescriptorLayoutBuilder
// ----------------------------------------------------------------------------------------------
void DescriptorLayoutBuilder::addBinding(const uint32_t binding, const VkDescriptorType type) {
    const VkDescriptorSetLayoutBinding newBind{
            .binding = binding,
            .descriptorType = type,
            .descriptorCount = 1,
    };

    bindings.push_back(newBind);
}

void DescriptorLayoutBuilder::clear() { bindings.clear(); }

VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags stageFlags, void *pNext,
                                                     VkDescriptorSetLayoutCreateFlags createFlags) {
    for (auto &b: bindings) {
        b.stageFlags |= stageFlags;
    }

    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.pNext = pNext;
    info.pBindings = bindings.data();
    info.bindingCount = static_cast<uint32_t>(bindings.size());
    info.flags = createFlags;

    VkDescriptorSetLayout layout;
    VK_CHECK((vkCreateDescriptorSetLayout(device, &info, nullptr, &layout)));

    return layout;
}

// ----------------------------------------------------------------------------------------------
// DescriptorAllocator
// ----------------------------------------------------------------------------------------------
void DescriptorAllocator::initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
    std::vector<VkDescriptorPoolSize> poolSizes;

    for (auto [type, ratio]: poolRatios) {
        poolSizes.push_back(
                VkDescriptorPoolSize{.type = type, .descriptorCount = static_cast<uint32_t>(ratio * maxSets)});
    }

    VkDescriptorPoolCreateInfo poolInfo{};

    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = 0;
    poolInfo.maxSets = maxSets;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
}

void DescriptorAllocator::clearDescriptors(const VkDevice device) { vkResetDescriptorPool(device, pool, 0); }

void DescriptorAllocator::destroyPool(const VkDevice device) { vkDestroyDescriptorPool(device, pool, nullptr); }

VkDescriptorSet DescriptorAllocator::allocate(const VkDevice device, const VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo info{};

    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.descriptorPool = pool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;

    VkDescriptorSet set;
    VK_CHECK(vkAllocateDescriptorSets(device, &info, &set));

    return set;
}
