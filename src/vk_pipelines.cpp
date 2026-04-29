//
// Created by JoonHo Son on 2026-04-29.
//

#include "vk_pipelines.h"
#include <fstream>
#include "vk_initializer.h"
#include "vk_types.h"

bool vkUtil::loadShaderModule(const char *filePath, VkDevice device, VkShaderModule *shaderModule) {
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    SPDLOG_DEBUG("Current dir : {}", std::filesystem::current_path().string());

    if (!file.is_open()) {
        SPDLOG_ERROR("Shader file open fail!");

        return false;
    }

    const size_t fileSize = file.tellg();

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    VkShaderModule _shaderModule;

    if (vkCreateShaderModule(device, &createInfo, nullptr, &_shaderModule) != VK_SUCCESS) {
        SPDLOG_ERROR("vkCreateShaderModule fail!");

        return false;
    }

    *shaderModule = _shaderModule;

    return true;
}
