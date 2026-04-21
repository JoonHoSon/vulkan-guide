//
// Created by JoonHo Son on 2026-04-20.
//

#ifndef VULKAN_GUIDE_VK_ENGINE_H
#define VULKAN_GUIDE_VK_ENGINE_H

#include "vk_types.h"
#include <spdlog/spdlog.h>

#if !defined(NDEBUG) || defined(_DEBUG)
#define DEBUG_BUILD
#endif

struct FrameData {
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
    VkSemaphore _swapChainSemaphore, _renderSemaphore;
    VkFence _renderFence;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine {
public:
    bool _isInitialized{false};
    int _frameNumber{0};
    bool stopRendering{false};
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkPhysicalDevice _chosenGPU;
    VkDevice _device;
    VkSurfaceKHR _surface;

    VkSwapchainKHR _swapChain;
    VkFormat _swapChainImageFormat;

    std::vector<VkImage> _swapChainImages;
    std::vector<VkImageView> _swapChainImageViews;
    VkExtent2D _swapChainExtent;
    FrameData _frames[FRAME_OVERLAP];

    VkQueue _graphicsQueue;
    uint32_t _graphicsQueueFamily;

    FrameData &getCurrentFrame() {
        return _frames[_frameNumber % FRAME_OVERLAP];
    }

    VkExtent2D _windowExtent{1700, 900};

    struct VulkanEngine &Get();

    struct SDL_Window *_window{nullptr};

    void init();

    void cleanup();

    void draw();

    void run();

private:
    void initVulkan();

    void initSwapChain();

    void initCommands();

    void initSyncStructures();

    void createSwapChain(uint32_t width, uint32_t height);

    void destroySwapChain();
};

#endif //VULKAN_GUIDE_VK_ENGINE_H
