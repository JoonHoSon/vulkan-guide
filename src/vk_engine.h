//
// Created by JoonHo Son on 2026-04-20.
//

#ifndef VULKAN_GUIDE_VK_ENGINE_H
#define VULKAN_GUIDE_VK_ENGINE_H

#include <ranges>
#include "vk_types.h"

struct DeletionQueue {
    std::deque<std::function<void()>> deleters;

    void pushFunction(std::function<void()> &&function) { deleters.push_back(function); }

    void flush() {
        // for (auto it = deletors.rbegin(); it != deletors.rend(); i++) {
        //     (*it)();
        // }

        for (auto &deleter: std::views::reverse(deleters)) {
            deleter();
        }

        deleters.clear();
    }
};

struct FrameData {
    VkCommandPool _commandPool;

    VkCommandBuffer _commandBuffer;

    // 렌더링 명령이 스왑체인 이미지 요청을 대기 하도록 한다.
    VkSemaphore _swapChainSemaphore;

    // 그리기가 끝났을 때 이미지를 OS에 표시하는 것을 제어한다.
    VkSemaphore _renderSemaphore;

    // 주어진 프레임의 그리기 명령이 끝날때까지 대기 하도록 한다.
    VkFence _renderFence;

    DeletionQueue _deletionQueue;
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

    FrameData &getCurrentFrame() { return _frames[_frameNumber % FRAME_OVERLAP]; }

    VkExtent2D _windowExtent{1700, 900};

    DeletionQueue _mainDeletionQueue;

    VmaAllocator _allocator;

    AllocatedImage _drawImage;

    VkExtent2D _drawExtent;

    VulkanEngine &get();

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

#endif // VULKAN_GUIDE_VK_ENGINE_H
