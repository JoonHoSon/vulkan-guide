//
// Created by JoonHo Son on 2026-04-20.
//

#ifndef VULKAN_GUIDE_VK_ENGINE_H
#define VULKAN_GUIDE_VK_ENGINE_H

#include "vk_types.h"

class VulkanEngine {
public:
    bool _isInitialized{false};
    int _frameNumber{0};
    bool stop_rendering{false};

    VkExtent2D _windowExtent{1700, 900};

    struct VulkanEngine &Get();

    struct SDL_Window *_window{nullptr};

    void init();

    void cleanup() const;

    void draw();

    void run();
};

#endif //VULKAN_GUIDE_VK_ENGINE_H
