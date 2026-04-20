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
