#include <iostream>
#include "vk_engine.h"

int main() {
#ifdef DEBUG_BUILD
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");
#else
    spdlog::set_level(spdlog::level::info);
#endif
    VulkanEngine engine;

    engine.init();
    engine.run();
    engine.cleanup();

    return 0;
}
