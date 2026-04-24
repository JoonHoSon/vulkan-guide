//
// Created by JoonHo Son on 2026-04-20.
//

#include "vk_engine.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <VkBootstrap.h>

#include <chrono>
#include <thread>

#include "vk_images.h"
#include "vk_initializer.h"
#include "vk_types.h"

constexpr bool bUseValidationLayers = true;

VulkanEngine *loadedEngine = nullptr;

VulkanEngine &VulkanEngine::Get() { return *loadedEngine; }

void VulkanEngine::init() {
    assert(loadedEngine == nullptr);

    loadedEngine = this;

    SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

    SDL_Init(SDL_INIT_VIDEO);

    _window = SDL_CreateWindow("Vulkan Engine", static_cast<int>(_windowExtent.width),
                               static_cast<int>(_windowExtent.height), flags);
    initVulkan();
    initSwapChain();
    initCommands();
    initSyncStructures();

    _isInitialized = true;
}

void VulkanEngine::cleanup() {
    if (_isInitialized) {
        vkDeviceWaitIdle(_device);

        for (const auto &frame: _frames) {
            vkDestroyCommandPool(_device, frame._commandPool, nullptr);

            vkDestroyFence(_device, frame._renderFence, nullptr);
            vkDestroySemaphore(_device, frame._renderSemaphore, nullptr);
            vkDestroySemaphore(_device, frame._swapChainSemaphore, nullptr);
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
    if (SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED) {
        return;
    }

    VK_CHECK(vkWaitForFences(_device, 1, &getCurrentFrame()._renderFence, true, 1'000'000'000));

    // Reset buffer
    VK_CHECK(vkResetFences(_device, 1, &getCurrentFrame()._renderFence));

    VkCommandBuffer command = getCurrentFrame()._commandBuffer;

    VK_CHECK(vkResetCommandBuffer(command, 0));

    uint32_t swapChainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(_device, _swapChain, 1'000'000'000, getCurrentFrame()._swapChainSemaphore, nullptr,
                                   &swapChainImageIndex));

    // Buffer 기록을 위한 준비
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    //  - 해당 버퍼가 단 한번만 제출될 것임을 명시
    //  - 약간의 성능 향상을 기대할 수 있음
    VkCommandBufferBeginInfo beginInfo = vkInit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // Buffer 기록 시작
    VK_CHECK(vkBeginCommandBuffer(command, &beginInfo));

    // Make the swap chain image into writeable mode before rendering
    vkUtil::transitionImage(command, _swapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clearValue;
    const float flash = std::abs(std::sin(static_cast<float>(_frameNumber) / 120.0f));
    clearValue = {{0.0f, 0.0f, flash, 1.0f}};

    const VkImageSubresourceRange clearRange = vkInit::imageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

    // Clear image
    vkCmdClearColorImage(command, _swapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1,
                         &clearRange);
    vkUtil::transitionImage(command, _swapChainImages[swapChainImageIndex], VK_IMAGE_LAYOUT_GENERAL,
                            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Finalize the command buffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(command));

    VkCommandBufferSubmitInfo bufferSubmitInfo = vkInit::commandSubmitInfo(command);
    VkSemaphoreSubmitInfo waitInfo = vkInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
                                                                 getCurrentFrame()._swapChainSemaphore);
    VkSemaphoreSubmitInfo signalInfo =
            vkInit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame()._renderSemaphore);
    VkSubmitInfo2 submitInfo = vkInit::submitInfo(&bufferSubmitInfo, &signalInfo, &waitInfo);

    VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submitInfo, getCurrentFrame()._renderFence));

    VkPresentInfoKHR presentInfo{};

    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &_swapChain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &getCurrentFrame()._renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapChainImageIndex;

    VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

    _frameNumber++;
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
#if !defined(_WIN32) && !defined(_WIN64)
    VkPhysicalDeviceVulkan14Features features14{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES};
    features14.dynamicRenderingLocalRead = true;

    // Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features features13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    features13.dynamicRendering = true;
    features13.synchronization2 = true;
#endif

    // Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features feature12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};

    feature12.bufferDeviceAddress = true;
    feature12.descriptorIndexing = true;

    vkb::PhysicalDeviceSelector selector{vkb_inst};

#if !defined(_WIN32) && !defined(_WIN64)
    selector = selector.set_minimum_version(1, 3)
                       .set_required_features_12(feature12)
                       .set_required_features_13(features13)
                       .set_surface(_surface);
    // .set_required_features_14(features14)
#else
    // windows(bootcapt) amd driver가 구버전임
    // Layer VK_LAYER_AMD_switchable_graphics uses API version 1.2 which is older than the application specified API
    // version of 1.4. May cause issues.
    selector = selector.set_minimum_version(1, 2).set_required_features_12(feature12).set_surface(_surface);
#endif

    // selector에서 오류 발생(Windows 11, MBPR2018 / Bootcamp)
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

    vkb::PhysicalDevice device = selector.select().value();

    vkb::DeviceBuilder deviceBuilder{device};
    vkb::Device vkbDevice = deviceBuilder.build().value();

    _device = vkbDevice.device;
    _chosenGPU = device.physical_device;

    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::initSwapChain() { createSwapChain(_windowExtent.width, _windowExtent.height); }

void VulkanEngine::initCommands() {
    VkCommandPoolCreateInfo command_pool_info = {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.pNext = nullptr;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_info.queueFamilyIndex = _graphicsQueueFamily;

    for (auto &frame: _frames) {
        VK_CHECK(vkCreateCommandPool(_device, &command_pool_info, nullptr, &frame._commandPool));

        VkCommandBufferAllocateInfo allocate_info = vkInit::commandBufferAllocateInfo(frame._commandPool, 1);

        VK_CHECK(vkAllocateCommandBuffers(_device, &allocate_info, &frame._commandBuffer));
    }
}

void VulkanEngine::initSyncStructures() {
    VkFenceCreateInfo fenceCreateInfo = vkInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkInit::semaphoreCreateInfo();

    for (auto &frame: _frames) {
        VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &frame._renderFence));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &frame._swapChainSemaphore));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &frame._renderSemaphore));
    }
}

void VulkanEngine::createSwapChain(uint32_t width, uint32_t height) {
    vkb::SwapchainBuilder builder{_chosenGPU, _device, _surface};
    _swapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain swapChain =
            builder.set_desired_format(VkSurfaceFormatKHR{.format = _swapChainImageFormat,
                                                          .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                    .set_desired_extent(width, height)
                    .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                    .build()
                    .value();
    _swapChainExtent = swapChain.extent;
    _swapChain = swapChain.swapchain;
    _swapChainImages = swapChain.get_images().value();
    _swapChainImageViews = swapChain.get_image_views().value();
}

void VulkanEngine::destroySwapChain() {
    vkDestroySwapchainKHR(_device, _swapChain, nullptr);

    for (const auto &view: _swapChainImageViews) {
        vkDestroyImageView(_device, view, nullptr);
    }
}
