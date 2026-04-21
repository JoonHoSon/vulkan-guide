# Library 설치

## Windows

```bash
C:\> vcpkg install vk-bootstrap
C:\> vcpkg install glm:x64-windows
C:\> vcpkg install fmt:x64-windows
C:\> vcpkg install sdl3:x64-windows
C:\> vcpkg install sdl3_image[jpeg,png,tiff,webp] --recurse # 이건 나중에
C:\> vcpkg integrate install
C:\> cmake -S . -B build\debug -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
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

``1s = 1000m = 1000000µ = 1000000000n``