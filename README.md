# Just another minecraft clone

## Building

I use CMake to build the project.
You can specify build type: `Release` or `Debug`.

### Linux

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. && make
```

The output will be either file `minecraft-clone`or file `minecraft-clone-debug`.

### Windows

I used MSVC compiler and visual studio generator.

```cmd
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

The output will be either file `minecraft-clone.exe`or file `minecraft-clone-debug.exe`.
