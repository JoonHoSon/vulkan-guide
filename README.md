# 문서 설명과 소스 분석이 어려워서 중단.

[Push Constants and new shaders](https://vkguide.dev/docs/ko/new_chapter_2/vulkan_pushconstants/)까지 진행하고 중단. [다른](https://vulkan-tutorial.com/Introduction) 튜토리얼을 기준으로 다시 시작

# Build

## Windows

```bash
cmake -S . -B build\debug -DCMAKE_BUILD_TYPE=debug -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake -S . -B build\release -DCMAKE_BUILD_TYPE=release -DCMAKE_TOOLCHAIN_FILE=c:\vcpkg\scripts\buildsystems\vcpkg.cmake

cmake --build .\build\debug
cmake --build .\build\release
```

## macOS / Linux

```bash
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=debug
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=release

cmake --build ./build/debug
cmake --build ./build/release
```

# Library 설치

## Windows

```bash
C:\> vcpkg install vk-bootstrap
C:\> vcpkg install glm:x64-windows
C:\> vcpkg install fmt:x64-windows
C:\> vcpkg install sdl3:x64-windows
C:\> vcpkg install sdl3_image[jpeg,png,tiff,webp] --recurse # 이건 나중에
C:\> vcpkg install vulkan-memory-allocator
C:\> vcpkg integrate install
```

## macOS

```bash
brew install sdl3 sdl3_image sdl2_mixer glm fmt spdlog
```

### vk-bootstrap

```bash
git clone git@github.com:charles-lunarg/vk-bootstrap.git
cd vk-bootstrap
cmake -S . -DCMAKE_BUILD_TYPE=Release -B build
cmake --build ./build
sudo cmake --install build
```

#### Header 파일 설치 경로

- `/usr/local/include/VKBootstrap*.*`

#### Static library 경로

- `/usr/local/lib/libvk-bootstrap.a`
- `/usr/loccal/lib/vk-bootstrap/vk-bootstrap*.cmake` => cmake 파일?

### Vulkan Memory Allocator

```bash
git clone git@github.com:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
cd VulkanMemoryAllocator
cmake -S . -B build
cmake --install build --prefix build/install
export VMA_LIB=/Users/joonho/dev/VulkanMemoryAllocator/build/install
```

export 처리를 위해 `.bashrc(or .zshrc)`에 등록

## Linux(e.g. Debian)

### 사전 준비

```bash
sudo apt install vulkan-tools libvulkan-dev libsdl3-image-dev libssl-dev glm-dev libfmt-dev libspdlog-dev \
ninja-build wayland-protocols liblz4-dev clang-format qt6-base-dev
```

### cmake 설치

```bash
wget https://github.com/Kitware/CMake/releases/download/v4.3.2/cmake-4.3.2.tar.gz
tar xzf cmake-4.3.2.tar.gz
cd cmake04.3.2
./bootstrap && make && sudo make install
```

### Vulkan SDK 설치

다운로드 받은 vulkansdk-linu-x86_64-<version>.tar.gz를 적당한 위치에 압축 해제.

```bash
tar xzf vulkansdk-linux-x86_64-<version>.tar.gz
mkdir ~/dev/tools && cd $_
cp -R ~/download/<version> ./vulkansdk<version>
cd ./vulkansdk<version>
./vulkansdk # sdk build
```

해당 위치에 있는 `setup-env.sh`를 활성화(e.g. **.bashrc** 마지막에 추가)

```bash
. "$HOME/dev/tools/vulkan<version>
```

### vk-bootstrap

```bash
cmake -S . -B build
sudo cmake --build build --target install
```

### VulkanMemoryAllocator

```bash
git clone git@github.com:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
cd VulkanMemoryAllocator
cmake -S . -B build
sudo cmake --build build --target install
```

# 정리

## `VkSemaphore`

GPU의 실행 순서 보장

```cpp
VkSemaphore task1;
VkSemaphore task2;

VkOperation opFrist;

opFirst.signalSemaphore = task1; // signal 수신용으로 task1 할당

VkDoSomething(opFirst);

VkOperation opSecond;

opSecond.signalSemaphore = task2; // signal 수신용으로 task2 할당
opSecond.waitSemaphore = task1;   // task1 완료시까지 대기

VkDoSomething(opSecond); // task1이 끝난 후 수행

VkOperation opThird;

opThird.waitSemaphore = task2; // task2 완료시까지 대기

VkDoSomething(opThird); // task2(opSecond)가 종료된 후 수행
```

## vkWaitForFences()

마지막 인자인 `timeout`은 nano 단위

```cpp
1s = 1000m = 1'000'000µ = 1'000'000'000n
```

## Pipeline에서의 Barrier 최적화 참고

`vkUtil::transitionImage`에서 사용된 `VkImageMemoryBarrier2`에 대한 보다 자세한 최적화 방법은
[Khronos Vulkan Documentation: Synchronization examples](https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples)
참고

## Image layout 관련

`VulkanEngine#draw()`에서 사용하는 `VK_IMAGE_LAYOUT_GENERAL`은 범용적인 레이아웃으로 이미지로부터 읽기/쓰기가 가능하다.<br>
Image layout에 대해 보다 자세한
설명은 [Vulkan Spec: image layouts](https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap12.html#resources-image-layouts)
참고

# 오류 발생

아래 오류는 모두 유효성 검사 활성화 여부(`bUseValidationLayers`)가 활성화 되었을 경우에 출력됨.<br>
[Swapchain Semaphore Reuse](https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html)문서 확인 필요.

## 온라인 소스(chapt-1)

```bash
[ERROR: Validation] - VUID-vkQueueSubmit-pSignalSemaphores-00067
vkQueueSubmit(): pSubmits[0].pSignalSemaphores[0] (VkSemaphore 0x110000000011) is being signaled by VkQueue 0xb7f12df58, but it may still be in use by VkSwapchainKHR 0x30000000003.
Most recently acquired image indices: [0], 1.
(Brackets mark the last use of VkSemaphore 0x110000000011 in a presentation operation.)
Swapchain image 0 was presented but was not re-acquired, so VkSemaphore 0x110000000011 may still be in use and cannot be safely reused with image index 1.
Vulkan insight: See https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html for details on swapchain semaphore reuse. Examples of possible approaches:
   a) Use a separate semaphore per swapchain image. Index these semaphores using the index of the acquired image.
   b) Consider the VK_KHR_swapchain_maintenance1 extension. It allows using a VkFence with the presentation operation.
The Vulkan spec states: Each binary semaphore element of the pSignalSemaphores member of any element of pSubmits must be unsignaled when the semaphore signal operation it defines is executed on the device (https://vulkan.lunarg.com/doc/view/1.4.341.0/mac/antora/spec/latest/chapters/cmdbuffers.html#VUID-vkQueueSubmit-pSignalSemaphores-00067)
[ERROR: Validation] - VUID-vkQueueSubmit-pSignalSemaphores-00067
vkQueueSubmit(): pSubmits[0].pSignalSemaphores[0] (VkSemaphore 0x110000000011) is being signaled by VkQueue 0xb7f12df58, but it may still be in use by VkSwapchainKHR 0x30000000003.
Most recently acquired image indices: 0, [1], 2.
(Brackets mark the last use of VkSemaphore 0x110000000011 in a presentation operation.)
Swapchain image 1 was presented but was not re-acquired, so VkSemaphore 0x110000000011 may still be in use and cannot be safely reused with image index 2.
Vulkan insight: See https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html for details on swapchain semaphore reuse. Examples of possible approaches:
   a) Use a separate semaphore per swapchain image. Index these semaphores using the index of the acquired image.
   b) Consider the VK_KHR_swapchain_maintenance1 extension. It allows using a VkFence with the presentation operation.
The Vulkan spec states: Each binary semaphore element of the pSignalSemaphores member of any element of pSubmits must be unsignaled when the semaphore signal operation it defines is executed on the device (https://vulkan.lunarg.com/doc/view/1.4.341.0/mac/antora/spec/latest/chapters/cmdbuffers.html#VUID-vkQueueSubmit-pSignalSemaphores-00067)
```

## chapt-1 기반 repository 소스

```bash
[ERROR: Validation] - VUID-vkQueueSubmit2-semaphore-03868
vkQueueSubmit2(): pSubmits[0].pSignalSemaphoreInfos[0].semaphore (VkSemaphore 0xe000000000e) is being signaled by VkQueue 0xc2f127e98, but it may still be in use by VkSwapchainKHR 0x30000000003.
Most recently acquired image indices: [0], 1, 2.
(Brackets mark the last use of VkSemaphore 0xe000000000e in a presentation operation.)
Swapchain image 0 was presented but was not re-acquired, so VkSemaphore 0xe000000000e may still be in use and cannot be safely reused with image index 2.
Vulkan insight: See https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html for details on swapchain semaphore reuse. Examples of possible approaches:
   a) Use a separate semaphore per swapchain image. Index these semaphores using the index of the acquired image.
   b) Consider the VK_KHR_swapchain_maintenance1 extension. It allows using a VkFence with the presentation operation.
The Vulkan spec states: The semaphore member of any binary semaphore element of the pSignalSemaphoreInfos member of any element of pSubmits must be unsignaled when the semaphore signal operation it defines is executed on the device (https://vulkan.lunarg.com/doc/view/1.4.341.0/mac/antora/spec/latest/chapters/cmdbuffers.html#VUID-vkQueueSubmit2-semaphore-03868)
[ERROR: Validation] - VUID-vkQueueSubmit2-semaphore-03868
vkQueueSubmit2(): pSubmits[0].pSignalSemaphoreInfos[0].semaphore (VkSemaphore 0x110000000011) is being signaled by VkQueue 0xc2f127e98, but it may still be in use by VkSwapchainKHR 0x30000000003.
Most recently acquired image indices: 0, [1], 2, 0.
(Brackets mark the last use of VkSemaphore 0x110000000011 in a presentation operation.)
Swapchain image 1 was presented but was not re-acquired, so VkSemaphore 0x110000000011 may still be in use and cannot be safely reused with image index 0.
Vulkan insight: See https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html for details on swapchain semaphore reuse. Examples of possible approaches:
   a) Use a separate semaphore per swapchain image. Index these semaphores using the index of the acquired image.
   b) Consider the VK_KHR_swapchain_maintenance1 extension. It allows using a VkFence with the presentation operation.
The Vulkan spec states: The semaphore member of any binary semaphore element of the pSignalSemaphoreInfos member of any element of pSubmits must be unsignaled when the semaphore signal operation it defines is executed on the device (https://vulkan.lunarg.com/doc/view/1.4.341.0/mac/antora/spec/latest/chapters/cmdbuffers.html#VUID-vkQueueSubmit2-semaphore-03868)
```
