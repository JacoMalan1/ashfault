<!-- Banner / Title -->
<h1 align="center">⚙️ Your Engine Name</h1>

<p align="center">
  <em>A modern game engine built with low-level power and scripting flexibility</em>
</p>

<!-- Badges -->
<p align="center">
  <img src="https://img.shields.io/badge/Graphics-Vulkan-red?style=for-the-badge&logo=vulkan&logoColor=white"/>
  <img src="https://img.shields.io/badge/Scripting-Lua-blue?style=for-the-badge&logo=lua&logoColor=white"/>
  <img src="https://img.shields.io/badge/Language-C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white"/>
</p>

<!-- Optional extras -->
<p align="center">
  <img src="https://img.shields.io/badge/Status-Active%20Development-success?style=flat-square"/>
  <img src="https://img.shields.io/badge/License-MIT-green?style=flat-square"/>
  <img src="https://img.shields.io/github/v/release/JacoMalan1/ashfault?include_prereleases&style=flat-square" alt="Latest Version"/>
  <img src="https://github.com/JacoMalan1/ashfault/actions/workflows/build.yml/badge.svg?event=push)](https://github.com/JacoMalan1/ashfault/actions/workflows/build.yml"/>
</p>

## 🛠️ Build Instructions

### 📦 Prerequisites

Make sure you have the following installed:

- **Vulkan SDK** (≥ 1.3)
- **CMake** ≥ 3.12
- A modern C++ compiler:
  - Clang ≥ 21
  - GCC ≥ 15
  - MSVC (latest version)

> ⚠️ Ensure your Vulkan SDK is properly set up and environment variables are configured (`VULKAN_SDK`, etc.)

---

### 📥 Clone the Repository

This project uses submodules, so make sure to clone with:

```bash
git clone --recursive https://github.com/JacoMalan1/ashfault.git
cd ashfault
```

---

### ⚙️ Build the Engine

A helper script is provided to configure and build the project:

#### Debug build (default)
```bash
./scripts/build.sh
```

#### Release build
```bash
./scripts/build.sh --release
```

---

### 🪟 Windows Notes

Run the build script using a compatible shell:
- Git Bash
- MSYS2
- WSL

Alternatively, you can generate and build manually using CMake (see below).

---

### 🔧 Manual CMake Build (Fallback)

```bash
mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

---

### 📁 Build Output

Compiled binaries and build files will be generated in the `build/` directory.
