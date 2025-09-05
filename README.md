# Just another minecraft clone

## Building

To manage packages in the project I use vspkg. When installing vcpkg you 
need to set the environment variable `VCPKG_ROOT`.

To build the project I use CMake
You can specify build type: `Release` or `Debug`.

### Linux
Install dependencies:
```bash
vcpkg install glfw3:x64-linux
vcpkg install cglm:x64-linux
vcpkg install sqlite3:x64-linux
vcpkg install zlib:x64-linux
```

Build:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DVCPKG_TARGET_TRIPLET=x64-linux .. && make
```

The output will be either file `minecraft-clone`or file `minecraft-clone-debug`.

### Windows

I used MSVC compiler and visual studio generator.

Install dependencies:
```cmd
%VCPKG_ROOT%\vcpkg.exe install glfw3:x64-windows
%VCPKG_ROOT%\vcpkg.exe install cglm:x64-windows
%VCPKG_ROOT%\vcpkg.exe install sqlite3:x64-windows
%VCPKG_ROOT%\vcpkg.exe install zlib:x64-windows
```

Build:
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build . --config Release
```

The output will be either file `minecraft-clone.exe`or file `minecraft-clone-debug.exe`.
