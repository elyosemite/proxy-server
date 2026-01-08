# Build & Run (Windows / MSVC)

This project uses CMake and vcpkg (manifest mode via `vcpkg.json`). On Windows, the most reliable setup is **MSVC** (Visual Studio toolchain) + the vcpkg triplet **`x64-windows`**.

## Prerequisites

- Visual Studio 2022 (or 2019) with:
  - **Desktop development with C++** workload
  - Windows 10/11 SDK
- CMake 3.23+ (required for presets support)
- vcpkg (already present in this repo under `vcpkg/`)

Optional (but recommended): Ninja (only if you want the Ninja generator).

## Quick Start (Using CMake Presets)

This project includes a `CMakePresets.json` with pre-configured build setups. This is the **recommended approach**.

From the repository root:

**1) Configure**

```bash
cmake --preset msvc-vs-debug
```

**2) Build**

```bash
cmake --build --preset msvc-vs-debug
```

**3) Run**

```bash
build-vs\Debug\proxy.exe 8080
```

For Release build:

```bash
cmake --preset msvc-vs-release
cmake --build --preset msvc-vs-release
build-vs\Release\proxy.exe 8080
```

---

## Alternative: Manual Configuration

If you prefer not to use presets or need to customize further:

### Option A: Visual Studio Generator

This uses the multi-config Visual Studio generator (Debug/Release are selected at build time).

From the repository root:

1) Configure

`cmake -S . -B build-vs -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="%CD%\vcpkg\scripts\buildsystems\vcpkg.cmake"`

2) Build (Debug)

`cmake --build build-vs --config Debug`

3) Run

`build-vs\Debug\proxy.exe 8080`

For Release, use:

`cmake --build build-vs --config Release`

and run:

`build-vs\Release\proxy.exe 8080`

### Option B: Ninja + MSVC (fast builds)

This uses Ninja but still compiles with MSVC (`cl.exe`).

**With preset:**

```bash
cmake --preset msvc-ninja-debug
cmake --build --preset msvc-ninja-debug
build-msvc-ninja\proxy.exe 8080
```

**Manual (without preset):**

1) Open a **Developer Command Prompt for VS** (so `cl` is available), then from the repo root:

2) Configure

```bash
cmake -S . -B build-msvc-ninja -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE="%CD%\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

3) Build

```bash
cmake --build build-msvc-ninja
```

4) Run

```bash
build-msvc-ninja\proxy.exe 8080
```

## Notes about vcpkg

- Because the repo has a `vcpkg.json`, vcpkg will automatically install the declared dependencies during CMake configure/build (manifest mode), as long as you pass `CMAKE_TOOLCHAIN_FILE`.
- For **MSVC**, the triplet should remain **`x64-windows`**.
- Avoid reusing the same build directory across different toolchains (e.g., MSVC vs MinGW). Use separate folders like `build-vs/`, `build-msvc-ninja/`, etc.

## Troubleshooting

### Linker errors mentioning `__imp__...` when using g++

That usually means you are mixing **MinGW (g++)** with vcpkg libraries built for **MSVC** (`x64-windows`). If you want g++/MinGW, use a MinGW triplet such as `x64-mingw-dynamic` and a separate build directory.

### CMake picked the wrong compiler

- For Visual Studio generator: reconfigure in a clean build folder (delete `build-vs/`).
- For Ninja + MSVC: make sure you are in a Developer Command Prompt and that `cl` is found via `where cl`.
