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

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "vk_pipelines.h"

constexpr bool bUseValidationLayers = true;

VulkanEngine *loadedEngine = nullptr;

VulkanEngine &VulkanEngine::get() {
    return *loadedEngine;
}

void VulkanEngine::init() {
    assert(loadedEngine == nullptr);

    loadedEngine = this;

    constexpr SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

    SDL_Init(SDL_INIT_VIDEO);

    if (const SDL_DisplayMode *displayMode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay())) {
        this->_windowExtent = VkExtent2D{
            .width = static_cast<uint32_t>(displayMode->w),
            .height = static_cast<uint32_t>(displayMode->h)
        };
    }

    _window = SDL_CreateWindow(
        "Vulkan Engine",
        static_cast<int>(_windowExtent.width),
        static_cast<int>(_windowExtent.height),
        flags
    );

    SDL_GetWindowPosition(_window, &_lastWindowPosition.x, &_lastWindowPosition.y);
    initVulkan();
    initSwapChain();
    initCommands();
    initSyncStructures();

    // 반드시 initSyncStructures() 다음에 호출
    initDescriptors();

    initPipelines();

    _isInitialized = true;

    // TODO(joonho): 2026-05-11 삭제
    // 최대 지원 버퍼 개수 확인
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_chosenGPU, _surface, &capabilities);

    SPDLOG_DEBUG("-------------------------------------------------------------------");
    SPDLOG_DEBUG("Hardware status");
    SPDLOG_DEBUG("-------------------------------------------------------------------");
    SPDLOG_DEBUG("Max image count : {}", capabilities.maxImageCount);
    SPDLOG_DEBUG("Min image count : {}", capabilities.minImageCount);
    SPDLOG_DEBUG("Max width       : {}", capabilities.maxImageExtent.width);
    SPDLOG_DEBUG("Max height      : {}", capabilities.maxImageExtent.height);
    SPDLOG_DEBUG("Min width       : {}", capabilities.minImageExtent.width);
    SPDLOG_DEBUG("Min height      : {}", capabilities.minImageExtent.height);
}

void VulkanEngine::cleanup() {
    if (_isInitialized) {
        VK_CHECK(vkDeviceWaitIdle(_device));

        for (auto &frame: _frames) {
            vkDestroyCommandPool(_device, frame._commandPool, nullptr);

            vkDestroyFence(_device, frame._renderFence, nullptr);
            vkDestroySemaphore(_device, frame._renderSemaphore, nullptr);
            vkDestroySemaphore(_device, frame._swapChainSemaphore, nullptr);

            frame._deletionQueue.flush();
        }

        _mainDeletionQueue.flush();

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

    getCurrentFrame()._deletionQueue.flush();

    // Reset buffer
    VK_CHECK(vkResetFences(_device, 1, &getCurrentFrame()._renderFence));

    VkCommandBuffer command = getCurrentFrame()._commandBuffer;

    VK_CHECK(vkResetCommandBuffer(command, 0));

    uint32_t swapChainImageIndex;

    VK_CHECK(
        vkAcquireNextImageKHR(_device, _swapChain, 1'000'000'000, getCurrentFrame()._swapChainSemaphore, nullptr, &
            swapChainImageIndex)
    );

    // Buffer 기록을 위한 준비
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    //  - 해당 버퍼가 단 한번만 제출될 것임을 명시
    //  - 약간의 성능 향상을 기대할 수 있음
    const VkCommandBufferBeginInfo beginInfo = vkInit::commandBufferBeginInfo(
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    );

    // Buffer 기록 시작
    VK_CHECK(vkBeginCommandBuffer(command, &beginInfo));

    _drawExtent.width = _drawImage.imageExtent.width;
    _drawExtent.height = _drawImage.imageExtent.height;

    vkUtil::transitionImage(command, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    drawBackground(command);

    vkUtil::transitionImage(command, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkUtil::transitionImage(
        command,
        _swapChainImages[swapChainImageIndex],
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    vkUtil::copyImageToImage(
        command,
        _drawImage.image,
        _swapChainImages[swapChainImageIndex],
        _drawExtent,
        _swapChainExtent
    );

    vkUtil::transitionImage(
        command,
        _swapChainImages[swapChainImageIndex],
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );

    VK_CHECK(vkEndCommandBuffer(command));

    /*
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
    */

    VkCommandBufferSubmitInfo bufferSubmitInfo = vkInit::commandSubmitInfo(command);
    VkSemaphoreSubmitInfo waitInfo = vkInit::semaphoreSubmitInfo(
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        getCurrentFrame()._swapChainSemaphore
    );
    VkSemaphoreSubmitInfo signalInfo = vkInit::semaphoreSubmitInfo(
        VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        getCurrentFrame()._renderSemaphore
    );
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

void VulkanEngine::drawBackground(const VkCommandBuffer buffer) const {
    // VkClearColorValue clearValue;
    // float flash = std::abs(std::sin(_frameNumber / 120.f));
    // clearValue = {{0.0f, 0.0f, flash, 1.0f}};
    //
    // VkImageSubresourceRange clearRange = vkInit::imageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    //
    // vkCmdClearColorImage(buffer, _drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_COMPUTE, _gradientPipeline);

    vkCmdBindDescriptorSets(
        buffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        _gradientPipelineLayout,
        0,
        1,
        &_drawImageDescriptors,
        0,
        nullptr
    );

    vkCmdDispatch(buffer, std::ceil(_drawExtent.width / 16.0), std::ceil(_drawExtent.height / 16.0), 1);
}

void VulkanEngine::run() {
    SDL_Event e;
    bool quit = false;
    bool changeSwapChain = false;

    while (!quit) {
        changeSwapChain = false;

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) quit = true;

            if (e.type == SDL_EVENT_WINDOW_MINIMIZED) {
                stopRendering = true;
            }

            if (_completeFirstCycle) {
                if (e.type == SDL_EVENT_WINDOW_RESIZED || e.type == SDL_EVENT_WINDOW_MOVED) {
                    stopRendering = true;
                } else {
                    stopRendering = false;
                }
            }

            if (e.type == SDL_EVENT_WINDOW_RESTORED) {
                stopRendering = false;
            }
        }

        if (stopRendering) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            continue;
        }

        if (!_completeFirstCycle) {
            _completeFirstCycle = true;
        }

        //
        // 현재 창의 크기, 위치를 확인
        // 만약 창의 크기나 위치가 변경되었다면 swapChain 재생성
        //
        if (const SDL_DisplayMode *displayMode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay())) {
            changeSwapChain = _windowExtent.width != displayMode->w || _windowExtent.height != displayMode->h;
        }

        if (!changeSwapChain) {
            WindowPosition currentPosition{0, 0};

            SDL_GetWindowPosition(_window, &currentPosition.x, &currentPosition.y);

            changeSwapChain = _lastWindowPosition.x != currentPosition.x || _lastWindowPosition.y != currentPosition.y;
        }

        // SPDLOG_DEBUG("change swap chain : {}", changeSwapChain);

        if (changeSwapChain) {
            // destroySwapChain();
            // createSwapChain(_windowExtent.width, _windowExtent.height);

            //
            // 참고 : https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation
            // vkDeviceWaitIdle(_device) // GPU 작업 완료시까지 대기
            // loop vkDestroyFrameBuffer
            //
        }

        draw();
    }
}

// ----------------------------------------------------------------------
// Private function
// ----------------------------------------------------------------------
void VulkanEngine::initVulkan() {
    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name("Example Vulkan Application").request_validation_layers(bUseValidationLayers).
                            use_default_debug_messenger().require_api_version(1, 3, 0).build();
    vkb::Instance vkb_inst = inst_ret.value();
    _instance = vkb_inst.instance;
    _debugMessenger = vkb_inst.debug_messenger;

    SDL_Vulkan_CreateSurface(_window, _instance, nullptr, &_surface);

#if !defined(_WIN32) && !defined(_WIN64)
    // Vulkan 1.4 features
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
    selector = selector.set_minimum_version(1, 3).set_required_features_12(feature12).
                        set_required_features_13(features13).set_required_features_14(features14).set_surface(_surface);
#else
    // windows(bootcamp) amd driver가 구버전임
    // Layer VK_LAYER_AMD_switchable_graphics uses API version 1.2 which is older than the application specified API
    // version of 1.4. May cause issues.
    selector = selector.set_minimum_version(1, 2).set_required_features_12(feature12).set_surface(_surface);
#endif

    // selector에서 오류 발생(Windows 11, MBPR2018 / Bootcamp)
    std::vector<vkb::PhysicalDevice> devices = selector.select_devices().value();

    for (const auto &dev: devices) {
        SPDLOG_DEBUG("-------------------------------------------------------");
        SPDLOG_DEBUG("Device name    : {}", dev.properties.deviceName);
        SPDLOG_DEBUG("API version    : {}", dev.properties.apiVersion);
        SPDLOG_DEBUG("Device ID      : {}", dev.properties.deviceID);
        SPDLOG_DEBUG("Device type    : {}", static_cast<int>(dev.properties.deviceType));
        SPDLOG_DEBUG("Driver version : {}", dev.properties.driverVersion);
        SPDLOG_DEBUG("Vendor ID      : {}", dev.properties.vendorID);
    }

    vkb::PhysicalDevice device = selector.select().value();

    vkb::DeviceBuilder deviceBuilder{device};
    vkb::Device vkbDevice = deviceBuilder.build().value();

    _device = vkbDevice.device;
    _chosenGPU = device.physical_device;

    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    VmaAllocatorCreateInfo allocatorInfo{};

    allocatorInfo.physicalDevice = _chosenGPU;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &_allocator);

    _mainDeletionQueue.pushFunction(
        [&] {
            vmaDestroyAllocator(_allocator);
        }
    );
}

void VulkanEngine::initSwapChain() {
    createSwapChain(_windowExtent.width, _windowExtent.height);

    const VkExtent3D drawImageExtent = {_windowExtent.width, _windowExtent.height, 1};
    _drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    _drawImage.imageExtent = drawImageExtent;

    VkImageUsageFlags drawImageUsages{};

    drawImageUsages = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageCreateInfo imageInfo = vkInit::imageCreateInfo(_drawImage.imageFormat, drawImageUsages, drawImageExtent);
    VmaAllocationCreateInfo imageAllocationInfo{};

    imageAllocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    imageAllocationInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(_allocator, &imageInfo, &imageAllocationInfo, &_drawImage.image, &_drawImage.allocation, nullptr);

    VkImageViewCreateInfo imageViewCreateInfo = vkInit::imageViewCreateInfo(
        _drawImage.imageFormat,
        _drawImage.image,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    VK_CHECK(vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &_drawImage.imageView));

    // add to deletion queues
    _mainDeletionQueue.pushFunction(
        [&] {
            vkDestroyImageView(_device, _drawImage.imageView, nullptr);
            vmaDestroyImage(_allocator, _drawImage.image, _drawImage.allocation);
        }
    );
}

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
    const VkFenceCreateInfo fenceCreateInfo = vkInit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    const VkSemaphoreCreateInfo semaphoreCreateInfo = vkInit::semaphoreCreateInfo();

    for (auto &frame: _frames) {
        VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &frame._renderFence));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &frame._swapChainSemaphore));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &frame._renderSemaphore));
    }
}

void VulkanEngine::createSwapChain(const uint32_t width, const uint32_t height) {
    vkb::SwapchainBuilder builder{_chosenGPU, _device, _surface};
    _swapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    // TODO(joonho): 2026-05-11 set_desired_min_image_count 확인
    // TODO(joonho): 2026-05-11 set_old_swapchain() 함수 확인
    vkb::Swapchain swapChain = builder.set_desired_format(
        VkSurfaceFormatKHR{.format = _swapChainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
    ).set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).set_desired_extent(width, height).add_image_usage_flags(
        VK_IMAGE_USAGE_TRANSFER_DST_BIT
    ).build().value();
    _swapChainExtent = swapChain.extent;
    _swapChain = swapChain.swapchain;
    _swapChainImages = swapChain.get_images().value();
    _swapChainImageViews = swapChain.get_image_views().value();
}

void VulkanEngine::destroySwapChain() const {
    vkDestroySwapchainKHR(_device, _swapChain, nullptr);

    for (const auto &view: _swapChainImageViews) {
        vkDestroyImageView(_device, view, nullptr);
    }
}

void VulkanEngine::initDescriptors() {
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

    globalAllocator.initPool(_device, 10, sizes); {
        DescriptorLayoutBuilder builder;

        builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

        _drawImageDescriptorLayout = builder.build(_device, VK_SHADER_STAGE_COMPUTE_BIT);
    }

    _drawImageDescriptors = globalAllocator.allocate(_device, _drawImageDescriptorLayout);

    VkDescriptorImageInfo imageInfo{};

    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.imageView = _drawImage.imageView;

    VkWriteDescriptorSet drawImageWrite{};

    drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    drawImageWrite.pNext = nullptr;
    drawImageWrite.dstBinding = 0;
    drawImageWrite.dstSet = _drawImageDescriptors;
    drawImageWrite.descriptorCount = 1;
    drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    drawImageWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(_device, 1, &drawImageWrite, 0, nullptr);

    _mainDeletionQueue.pushFunction(
        [&] {
            globalAllocator.destroyPool(_device);
            vkDestroyDescriptorSetLayout(_device, _drawImageDescriptorLayout, nullptr);
        }
    );
}

void VulkanEngine::initPipelines() {
    initBackgroundPipelines();
}

void VulkanEngine::initBackgroundPipelines() {
    VkPipelineLayoutCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.pSetLayouts = &_drawImageDescriptorLayout;
    createInfo.setLayoutCount = 1;

    VK_CHECK(vkCreatePipelineLayout(_device, &createInfo, nullptr, &_gradientPipelineLayout));

    VkShaderModule shaderModule{};

    if (!vkUtil::loadShaderModule("./shaders/gradient.comp.spv", _device, &shaderModule)) {
        SPDLOG_ERROR("Error when building the compute shader.");
        exit(1);
    }

    VkPipelineShaderStageCreateInfo shaderCreateInfo{};

    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderCreateInfo.pNext = nullptr;
    shaderCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderCreateInfo.module = shaderModule;
    shaderCreateInfo.pName = "main";

    VkComputePipelineCreateInfo computeCreateInfo{};

    computeCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computeCreateInfo.pNext = nullptr;
    computeCreateInfo.layout = _gradientPipelineLayout;
    computeCreateInfo.stage = shaderCreateInfo;

    VK_CHECK(vkCreateComputePipelines(_device, VK_NULL_HANDLE, 1, &computeCreateInfo, nullptr, &_gradientPipeline));

    vkDestroyShaderModule(_device, shaderModule, nullptr);

    _mainDeletionQueue.pushFunction(
        [&] {
            vkDestroyPipelineLayout(_device, _gradientPipelineLayout, nullptr);
            vkDestroyPipeline(_device, _gradientPipeline, nullptr);
        }
    );
}
