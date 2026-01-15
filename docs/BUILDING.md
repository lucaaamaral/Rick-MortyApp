# Building Rick and Morty Explorer

This document covers the build system architecture, prerequisites, and procedures for building the application.

## Overview

The project uses a **CMake superbuild pattern** with `ExternalProject_Add` to manage dependencies. All dependencies (Qt6, curl, glog, fonts) are built from source and cached locally.

**Supported Platforms:**
| OS | Architecture | Status |
|----|--------------|--------|
| Linux | x86_64 | Supported |
| Linux | arm64 | Supported |
| Windows | x86_64 | Supported (cross-compilation) |
| Windows | arm64 | Supported (cross-compilation) |

## Prerequisites

### Option A: Docker (Recommended)

Docker provides a consistent build environment with all dependencies pre-configured.

```bash
# Install Docker
# See: https://docs.docker.com/get-docker/

# Verify docker-compose is available
docker compose version
```

### Option B: Native Build

For building without Docker, install the following:

**Build Tools:**
- CMake 3.16+
- GCC/G++ 10+ (or cross-compiler)
- Ninja or Make
- Git, Perl, Python3

**Qt6 Dependencies (Debian/Ubuntu):**
```bash
sudo apt install \
    libxcb1-dev libxcb-cursor-dev libxcb-glx0-dev libxcb-icccm4-dev \
    libxcb-image0-dev libxcb-keysyms1-dev libxcb-randr0-dev \
    libxcb-render-util0-dev libxcb-shape0-dev libxcb-shm0-dev \
    libxcb-sync-dev libxcb-xfixes0-dev libxcb-xinerama0-dev \
    libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev \
    libx11-dev libx11-xcb-dev libxext-dev libxrender-dev \
    libgl1-mesa-dev libegl1-mesa-dev libgles2-mesa-dev \
    libdrm-dev libgbm-dev libwayland-dev \
    libfontconfig1-dev libfreetype-dev \
    libpng-dev libjpeg-dev libssl-dev \
    libinput-dev libmtdev-dev libts-dev libdbus-1-dev zlib1g-dev
```

**Cross-Compilers (for cross-compilation):**
```bash
# Linux ARM64 from x86_64 host
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# Linux x86_64 from ARM64 host
sudo apt install gcc-x86-64-linux-gnu g++-x86-64-linux-gnu

# Windows x86_64
sudo apt install mingw-w64

# Windows ARM64 (requires additional setup)
# See: https://github.com/AZO234/mingw-cross
```

**AppImage Tools (Linux only):**
```bash
sudo apt install fuse libfuse2
# appimagetool is downloaded automatically during build
```

## Directory Structure

```
.build/                          # CMake build artifacts
├── linux-arm64/                 # ARM64 Linux build
├── linux-x86_64/                # x86_64 Linux build
├── windows-arm64/               # ARM64 Windows build
└── windows-x86_64/              # x86_64 Windows build

.lib-prebuilt/                   # Cached dependencies (persist across builds)
├── linux-arm64/
│   ├── qt6/                     # Qt6 installation
│   ├── curl/                    # libcurl installation
│   ├── glog/                    # glog installation
│   ├── fonts/                   # Font files
│   ├── .qt-stamp                # Qt build completion marker
│   ├── .curl-stamp              # curl build completion marker
│   └── .glog-stamp              # glog build completion marker
└── ...

.distribute/                     # Distribution bundles
├── linux-arm64/
│   ├── AppDir/                  # AppImage directory structure
│   └── ...
├── RickAndMortyViewer-aarch64.AppImage
└── RickAndMortyViewer-x86_64.AppImage
```

## Build Workflow

### Using Docker (Recommended)

Docker services are defined in `docker-compose.yml`:

| Service | Purpose |
|---------|---------|
| `deps-arm64` / `deps-amd64` | Build Qt6 + curl + glog + fonts |
| `app-arm64` / `app-amd64` | Build application only |
| `distribute-arm64` / `distribute-amd64` | Create distribution bundle |
| `appimage-arm64` / `appimage-amd64` | Create AppImage |

**First-time Build (2-3 hours for Qt):**
```bash
# Build dependencies first
docker compose run --rm deps-arm64

# Build app + distribution + AppImage
docker compose run --rm appimage-arm64
```

**Incremental Build (uses cached dependencies):**
```bash
# Just rebuild app and create AppImage
docker compose run --rm appimage-arm64
```

**Clean Rebuild:**
```bash
# Remove build artifacts (keeps cached dependencies)
rm -rf .build/ .distribute/

# Remove everything including cached Qt/curl/glog
rm -rf .build/ .distribute/ .lib-prebuilt/
```

### Native Build (Without Docker)

**Configure and Build:**
```bash
# Auto-detect platform
cmake -B .build/linux-arm64

# Or specify platform explicitly
cmake -B .build/linux-arm64 -DTARGET_PLATFORM=linux-arm64

# Build all (dependencies + application)
cmake --build .build/linux-arm64

# Build specific targets
cmake --build .build/linux-arm64 --target qt6_external     # Qt only
cmake --build .build/linux-arm64 --target rick_and_morty_viewer  # App only
```

**Create Distribution:**
```bash
# Create distribution bundle
cmake --build .build/linux-arm64 --target distribute

# Create AppImage (Linux only)
cmake --build .build/linux-arm64 --target appimage
```

**Run the Application:**
```bash
# Using the distribution bundle
./.distribute/linux-arm64/run.sh

# Using the AppImage
./.distribute/RickAndMortyViewer-aarch64.AppImage
```

## Cross-Compilation

Cross-compilation is supported via CMake toolchain files in `cmake/toolchains/`.

### Toolchain Files

| File | Target | Cross-Compiler |
|------|--------|----------------|
| `linux-x86_64.cmake` | Linux x86_64 | `x86_64-linux-gnu-gcc` |
| `linux-arm64.cmake` | Linux ARM64 | `aarch64-linux-gnu-gcc` |
| `windows-x86_64.cmake` | Windows x86_64 | `x86_64-w64-mingw32-gcc` |
| `windows-arm64.cmake` | Windows ARM64 | `aarch64-w64-mingw32-gcc` |

### Cross-Compile Example

```bash
# From x86_64 host, build for ARM64 Linux
cmake -B .build/linux-arm64 -DTARGET_PLATFORM=linux-arm64
cmake --build .build/linux-arm64

# From Linux, build for Windows x86_64
cmake -B .build/windows-x86_64 -DTARGET_PLATFORM=windows-x86_64
cmake --build .build/windows-x86_64
```

### Qt Cross-Compilation

Qt cross-compilation requires a **host Qt** installation first (for moc, rcc, uic tools):

1. Build Qt for your host platform first
2. The superbuild will automatically use the host Qt when cross-compiling

```bash
# Step 1: Build host Qt (on x86_64)
cmake -B .build/linux-x86_64 -DTARGET_PLATFORM=linux-x86_64
cmake --build .build/linux-x86_64 --target qt6_external

# Step 2: Cross-compile for ARM64 (uses host Qt automatically)
cmake -B .build/linux-arm64 -DTARGET_PLATFORM=linux-arm64
cmake --build .build/linux-arm64
```

## CMake Targets

| Target | Description |
|--------|-------------|
| `qt6_external` | Build Qt6 from source |
| `curl_external` | Build libcurl (static) |
| `glog_external` | Build Google logging (static) |
| `bangers_font` | Download Bangers font |
| `creepster_font` | Download Creepster font |
| `nunito_font` | Download Nunito font |
| `noto_emoji` | Download Noto Color Emoji |
| `rick_and_morty_viewer` | Build main application |
| `distribute` | Create distribution bundle |
| `appimage` | Create AppImage (Linux only) |

## Caching Strategy

Dependencies are cached to avoid rebuilding:

| Dependency | Cache Location | Marker |
|------------|----------------|--------|
| Qt6 | `.lib-prebuilt/<platform>/qt6/` | `.qt-stamp` |
| curl | `.lib-prebuilt/<platform>/curl/` | `.curl-stamp` |
| glog | `.lib-prebuilt/<platform>/glog/` | `.glog-stamp` |
| Fonts | `.lib-prebuilt/<platform>/fonts/` | Individual files |

**Force Rebuild of Dependencies:**
```bash
# Force Qt rebuild
rm .lib-prebuilt/linux-arm64/.qt-stamp

# Force curl rebuild
rm .lib-prebuilt/linux-arm64/.curl-stamp

# Force font re-download
rm .lib-prebuilt/linux-arm64/fonts/Bangers-Regular.ttf
```

## Platform-Specific Notes

### Linux

- Supports **X11 (XCB)** and **Wayland** display backends
- Qt auto-detects available backends at build time
- AppImage bundles all dependencies for portability

### Windows

- Cross-compiled from Linux using MinGW
- Uses `win32-g++` Qt platform
- Produces `.exe` binary and DLL dependencies
- No AppImage (Windows uses different packaging)

## Troubleshooting

### Permission Issues (Docker)

Files created by Docker are owned by root. To fix:
```bash
sudo chown -R $(whoami):$(whoami) .build/ .distribute/ .lib-prebuilt/
```

### Qt Build Failures

Common issues:
- **Missing XCB libraries**: Install all `libxcb-*-dev` packages
- **OpenGL issues**: Install `libgl1-mesa-dev`, `libegl1-mesa-dev`
- **Wayland issues**: Install `libwayland-dev`

### Cross-Compilation Issues

- Ensure cross-compiler is installed and in PATH
- For Qt cross-compilation, build host Qt first
- Check that target libraries are available in sysroot

### AppImage Issues

- Requires FUSE: `sudo apt install fuse libfuse2`
- On systems without FUSE, extract and run:
  ```bash
  ./RickAndMortyViewer-aarch64.AppImage --appimage-extract
  ./squashfs-root/AppRun
  ```

## Build Times (Approximate)

| Component | First Build | Incremental |
|-----------|-------------|-------------|
| Qt6 | 2-3 hours | Cached |
| curl | 2-3 minutes | Cached |
| glog | 1-2 minutes | Cached |
| Application | 1-2 minutes | 10-30 seconds |
| Distribution | 30 seconds | 30 seconds |
| AppImage | 30 seconds | 30 seconds |

## See Also

- [API Documentation](API.md) - Rick and Morty API integration
- [Design Documentation](DESIGN.md) - Architecture overview
- [UI Design](UI_DESIGN.md) - User interface specifications
