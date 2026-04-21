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
