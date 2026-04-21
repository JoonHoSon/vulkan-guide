//
// Created by JoonHo Son on 2026-04-20.
//

#include "vk_engine.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <VkBootstrap.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "vk_initializer.h"
#include "vk_types.h"

constexpr bool bUseValidationLayers = false;

VulkanEngine *loadedEngine = nullptr;

VulkanEngine &VulkanEngine::Get() { return *loadedEngine; }

void VulkanEngine::init() {
    assert(loadedEngine == nullptr);

    loadedEngine = this;

    SDL_Init(SDL_INIT_VIDEO);

    _window = SDL_CreateWindow("Vulkan Engine", _windowExtent.width, _windowExtent.height, SDL_WINDOW_VULKAN);
    initVulkan();
    initSwapChain();
    initCommands();
    initSyncStructures();

    _isInitialized = true;
}

void VulkanEngine::cleanup() {
    if (_isInitialized) {
        vkDeviceWaitIdle(_device);

        for (int i = 0; i < FRAME_OVERLAP; i++) {
            vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
        }

        destroySwapChain();

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyDevice(_device, nullptr);

        vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
        vkDestroyInstance(_instance, nullptr);

        SDL_DestroyWindow(_window);
    }
}

void VulkanEngine::draw() {
    VK_CHECK(vkWaitForFences(_device, 1, &getCurrentFrame()._renderFence, true, 1000000000));
    VK_CHECK(vkResetFences(_device, 1, &getCurrentFrame()._renderFence));
    uint32_t swapChainImageIndex;

    VK_CHECK(
        vkAcquireNextImageKHR(_device, _swapChain, 1000000000, getCurrentFrame()._swapChainSemaphore, nullptr, &
            swapChainImageIndex));
}

void VulkanEngine::run() {
    SDL_Event e;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT)
                quit = true;

            if (e.type == SDL_EVENT_WINDOW_MINIMIZED) {
                stopRendering = true;
            }

            if (e.type == SDL_EVENT_WINDOW_RESTORED) {
                stopRendering = false;
            }
        }

        if (stopRendering) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            continue;
        }

        draw();
    }
}

// ----------------------------------------------------------------------
// Private function
// ----------------------------------------------------------------------
void VulkanEngine::initVulkan() {
    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name("Example Vulkan Application")
                           .request_validation_layers(bUseValidationLayers)
                           .use_default_debug_messenger()
                           .require_api_version(1, 3, 0)
                           .build();
    vkb::Instance vkb_inst = inst_ret.value();
    _instance = vkb_inst.instance;
    _debugMessenger = vkb_inst.debug_messenger;

    // TODO(joonho): 2026-04-21 allocator 확인 필요
    SDL_Vulkan_CreateSurface(_window, _instance, nullptr, &_surface);

    // FIXME(joonho): 2026-04-21 macOS에서 1.4 feature 설정 시 오류 발생함
    // Vulkan 1.4 features
    VkPhysicalDeviceVulkan14Features features14{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES};
    features14.dynamicRenderingLocalRead = true;

    // Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    // Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features feature12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};

    feature12.bufferDeviceAddress = true;
    feature12.descriptorIndexing = true;

    vkb::PhysicalDeviceSelector selector{vkb_inst};
    selector = selector.set_minimum_version(1, 3)
                       .set_required_features_12(feature12)
                       .set_required_features_13(features13)
                       // .set_required_features_14(features14)
                       .set_surface(_surface);

    std::vector<vkb::PhysicalDevice> devices = selector.select_devices().value();

    for (const auto &dev: devices) {
        spdlog::debug("-------------------------------------------------------");
        spdlog::info("Device name    : {}", dev.properties.deviceName);
        spdlog::debug("API version    : {}", dev.properties.apiVersion);
        spdlog::debug("Device ID      : {}", dev.properties.deviceID);
        spdlog::debug("Device type    : {}", static_cast<int>(dev.properties.deviceType));
        spdlog::debug("Driver version : {}", dev.properties.driverVersion);
        spdlog::debug("Vendor ID      : {}", dev.properties.vendorID);
    }

    vkb::PhysicalDevice device = selector.set_surface(_surface).select().value();

    vkb::DeviceBuilder deviceBuilder{device};
    vkb::Device vkbDevice = deviceBuilder.build().value();

    _device = vkbDevice.device;
    _chosenGPU = device.physical_device;

    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::initSwapChain() {
    createSwapChain(_windowExtent.width, _windowExtent.height);
}

void VulkanEngine::initCommands() {
    VkCommandPoolCreateInfo command_pool_info = {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.pNext = nullptr;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_info.queueFamilyIndex = _graphicsQueueFamily;

    for (int i = 0; i < FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateCommandPool(_device, &command_pool_info, nullptr, &_frames[i]._commandPool));

        VkCommandBufferAllocateInfo allocate_info = vkInit::commandBufferAllocateInfo(_frames[i]._commandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(_device, &allocate_info, &_frames[i]._commandBuffer));
    }
}

void VulkanEngine::initSyncStructures() {
    VkFenceCreateInfo fenceCreateInfo = vkInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkInit::semaphoreCreateInfo();

    for (int i = 0; i < FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._swapChainSemaphore));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));
    }
}

void VulkanEngine::createSwapChain(uint32_t width, uint32_t height) {
    vkb::SwapchainBuilder builder{_chosenGPU, _device, _surface};
    _swapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain swapChain = builder.set_desired_format(VkSurfaceFormatKHR{
                                          .format = _swapChainImageFormat,
                                          .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                                      })
                                      .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                      .set_desired_extent(width, height)
                                      .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                      .build().value();
    _swapChainExtent = swapChain.extent;
    _swapChain = swapChain.swapchain;
    _swapChainImages = swapChain.get_images().value();
    _swapChainImageViews = swapChain.get_image_views().value();
}

void VulkanEngine::destroySwapChain() {
    vkDestroySwapchainKHR(_device, _swapChain, nullptr);

    for (int i = 0; i < _swapChainImageViews.size(); i++) {
        vkDestroyImageView(_device, _swapChainImageViews[i], nullptr);
    }
}
