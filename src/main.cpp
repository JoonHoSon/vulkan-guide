#include <iostream>
#include "vk_engine.h"

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in
// the gutter.

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
    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you
    // can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}
