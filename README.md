# BVHView_Cpp

> `BVHView_Cpp` is a C++17 rewrite of
> [orangeduck/BVHView](https://github.com/orangeduck/BVHView), intended as a
> clean baseline for a more general animation viewer.

*Currently, only Windows is supported.*

- The rewrite preserves the original BVH viewer behavior and the existing
  `raylib`/`raygui` dependency setup, while splitting the former single-file C
  implementation into focused modules.
- It provides a reference baseline for a future, separate `ASCII_Anim_Viewer`
  project.
  - The goal is to support more ASCII-based animation file types, such as `SMD`.

It also adds resizable-window support, `F11` fullscreen switching, a
`1280 x 720` minimum window size, and proportional UI scaling.

## Requirements

- Windows with `MinGW` or `MSVC`
- `CMake` 3.22 or newer
- A `C++17` compiler supported by CMake
- `Git`

## Build

raylib and raygui are pinned Git submodules under `external/`. Clone and build
the project with any suitable CMake generator:

```powershell
git clone --recursive <repository-url>
cd BVHView_Cpp
cmake -S . -B build
cmake --build build --config Release
```

If you need to specify the build system, use commands like:

```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_CXX_COMPILER=g++
cmake --build build
```

Single-config generators such as Ninja use `Release` by default. To build a
Debug configuration, specify it explicitly:

```powershell
cmake -S . -B build-debug -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_CXX_COMPILER=g++
cmake --build build-debug
```

For an existing clone without initialized submodules:

```powershell
git submodule update --init --recursive
```

CMake builds the bundled raylib together with the application. No separate
raylib build step is required.

After building, use the executable in the `app` directory.

## Usage

Open a `.bvh` file with the in-app file dialog, drag files into the window, or
pass one or more file paths on the command line:

```powershell
.\app\bvhview.exe animation.bvh
```

Useful controls:

| Input                       | Action            |
| --------------------------- | ----------------- |
| `Ctrl + left mouse drag`  | Orbit camera      |
| `Ctrl + right mouse drag` | Pan camera        |
| Mouse wheel                 | Zoom              |
| `H`                       | Hide or show UI   |
| `F11`                     | Toggle fullscreen |

## License

MIT. This rewrite is based on
[orangeduck/BVHView](https://github.com/orangeduck/BVHView), which is released
under the MIT License.
