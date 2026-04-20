//
// Created by JoonHo Son on 2026-04-20.
//

#include "vk_engine.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "vk_initializer.h"
#include "vk_types.h"
#include <chrono>
#include <thread>

constexpr bool bUseValidationLayers = false;

VulkanEngine *loadedEngine = nullptr;

VulkanEngine &VulkanEngine::Get() {
    return *loadedEngine;
}

void VulkanEngine::init() {
    assert(loadedEngine == nullptr);

    loadedEngine = this;

    SDL_Init(SDL_INIT_VIDEO);
    constexpr auto flags = (SDL_WindowFlags) (SDL_WINDOW_VULKAN);

    _window = SDL_CreateWindow("Vulkan Engine",
                               _windowExtent.width,
                               _windowExtent.height, flags);
    _isInitialized = true;
}

void VulkanEngine::cleanup() const {
    if (_isInitialized)
        SDL_DestroyWindow(_window);
}

void VulkanEngine::draw() {
}

void VulkanEngine::run() {
    SDL_Event e;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) quit = true;

            if (e.type == SDL_EVENT_WINDOW_MINIMIZED) {
                stop_rendering = true;
            }
            
            if (e.type == SDL_EVENT_WINDOW_RESTORED) {
                stop_rendering = false;
            }
        }

        if (stop_rendering) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            continue;
        }

        draw();
    }
}
