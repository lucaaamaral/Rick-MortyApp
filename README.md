# Rick and Morty Viewer

A cross-platform desktop application for browsing episodes and characters from the Rick and Morty universe. Built with C++, Qt6/QML, and featuring a Rick and Morty themed UI with custom typography and animations.

![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-blue)
![Qt](https://img.shields.io/badge/Qt-6.8-green)
![C++](https://img.shields.io/badge/C%2B%2B-17-orange)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

## Features

- Browse all episodes organized by season
- View character details with images and metadata
- Rick and Morty themed UI with custom fonts (Bangers, Creepster, Nunito)
- Animated portal spinner and star field background
- Cross-platform support (Linux x86_64/ARM64, Windows x86_64/ARM64)
- Portable AppImage distribution for Linux

## Screenshots

*Application screenshots would go here*

## Quick Start

### Using Docker (Recommended)

The easiest way to build is using Docker, which provides a consistent build environment.

```bash
# Clone the repository
git clone https://github.com/yourusername/rick-and-morty-viewer.git
cd rick-and-morty-viewer

# Build everything (first run takes 2-3 hours for Qt)
./build.sh release

# Run the application
./.distribute/RickAndMortyViewer-*.AppImage
```

### Using build.sh

The `build.sh` script provides a convenient interface for common build tasks:

```bash
./build.sh deps      # Build Qt + dependencies (~2-3 hours first time)
./build.sh app       # Build application only (~1-2 min)
./build.sh distribute # Create distribution bundle
./build.sh appimage  # Create AppImage (Linux only)
./build.sh release   # Full build: deps + app + distribute + appimage
./build.sh status    # Show build status
./build.sh clean     # Clean build artifacts (keeps cached deps)
./build.sh clean-all # Clean everything including cached deps
```

## Prerequisites

### Option A: Docker (Recommended)

Docker provides all build dependencies in a container.

```bash
# Install Docker
# See: https://docs.docker.com/get-docker/

# Verify installation
docker compose version
```

### Option B: Native Build

For building without Docker, install the following dependencies:

**Build Tools:**
- CMake 3.16+
- GCC/G++ 10+ or Clang
- Ninja or Make
- Git, Perl, Python3

**Qt6 Dependencies (Debian/Ubuntu):**

```bash
sudo apt install \
    build-essential cmake ninja-build git pkg-config perl python3 \
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

**For AppImage creation:**

```bash
sudo apt install fuse libfuse2
```

## Building

### Docker Build

Docker Compose services handle all build steps:

```bash
# Build dependencies (cached after first build)
docker compose run --rm deps-linux-x86_64   # For Linux x86_64
docker compose run --rm deps-linux-arm64    # For Linux ARM64
docker compose run --rm deps-windows-x86_64 # For Windows x86_64
docker compose run --rm deps-windows-arm64  # For Windows ARM64

# Build application
docker compose run --rm app-linux-x86_64
docker compose run --rm app-windows-x86_64

# Create distribution
docker compose run --rm distribute-linux-x86_64
docker compose run --rm appimage-linux-x86_64
```

### Manual CMake Build

```bash
# Configure (auto-detects platform)
cmake -B .build/linux-x86_64

# Or specify platform explicitly
cmake -B .build/linux-x86_64 -DTARGET_PLATFORM=linux-x86_64

# Build all (dependencies + application)
cmake --build .build/linux-x86_64

# Build specific targets
cmake --build .build/linux-x86_64 --target qt6_external      # Qt only
cmake --build .build/linux-x86_64 --target rick_and_morty_viewer  # App only
cmake --build .build/linux-x86_64 --target distribute        # Create bundle
cmake --build .build/linux-x86_64 --target appimage          # Create AppImage
```

### Running the Application

```bash
# From distribution bundle
./.distribute/linux-x86_64/run.sh

# Using AppImage
./.distribute/RickAndMortyViewer-x86_64.AppImage

# AppImage without FUSE
./.distribute/RickAndMortyViewer-x86_64.AppImage --appimage-extract
./squashfs-root/AppRun
```

## Cross-Compilation

The project supports cross-compilation via CMake toolchain files:

| Target | Toolchain File | Cross-Compiler |
|--------|----------------|----------------|
| Linux x86_64 | `cmake/toolchains/linux-x86_64.cmake` | `x86_64-linux-gnu-gcc` |
| Linux ARM64 | `cmake/toolchains/linux-arm64.cmake` | `aarch64-linux-gnu-gcc` |
| Windows x86_64 | `cmake/toolchains/windows-x86_64.cmake` | `x86_64-w64-mingw32-gcc` |
| Windows ARM64 | `cmake/toolchains/windows-arm64.cmake` | `aarch64-w64-mingw32-gcc` |

```bash
# Cross-compile for ARM64 from x86_64 host
cmake -B .build/linux-arm64 -DTARGET_PLATFORM=linux-arm64
cmake --build .build/linux-arm64

# Cross-compile for Windows
cmake -B .build/windows-x86_64 -DTARGET_PLATFORM=windows-x86_64
cmake --build .build/windows-x86_64
```

## Testing

The project includes a comprehensive test suite using Google Test.

```bash
# Build and run tests via build.sh
./build.sh test

# Or using CMake directly
cmake -B .build/tests -DBUILD_TESTS=ON
cmake --build .build/tests --target tests
cd .build/tests/tests-build && ctest --output-on-failure
```

Test categories:
- **Unit tests**: URL extraction, JSON parsing, model validation
- **Integration tests**: API client with mock HTTP responses
- **Property tests**: RapidCheck-based generative testing

## Project Structure

```
rick-and-morty-viewer/
├── src/
│   ├── core/           # Backend: API client, data models, observer pattern
│   │   ├── ApiClient.cpp/h      # HTTP client for Rick and Morty API
│   │   ├── DataStore.cpp/h      # Cached data with observer notifications
│   │   ├── Models.h             # Character, Episode, Location structs
│   │   ├── Observer.h           # IDataObserver interface
│   │   ├── IHttpClient.h        # HTTP client interface for DI
│   │   └── CurlHttpClient.cpp/h # CURL-based HTTP implementation
│   ├── ui/             # Qt models and QML bridge
│   │   ├── QmlBridge.cpp/h      # C++ to QML interface
│   │   ├── EpisodeModel.cpp/h   # QAbstractListModel for episodes
│   │   └── CharacterModel.cpp/h # QAbstractListModel for characters
│   ├── main.cpp        # Application entry point
│   └── MainWindow.cpp/h # Main window setup
├── resources/
│   ├── qml/            # QML UI components
│   │   ├── Main.qml             # Root component
│   │   ├── Theme.qml            # Colors and styling
│   │   └── components/          # Reusable UI components
│   └── fonts/          # Custom fonts (Bangers, Creepster, Nunito)
├── tests/              # Test suite
│   ├── unit/           # Unit tests
│   ├── integration/    # Integration tests
│   ├── fakes/          # FakeHttpClient for testing
│   └── mocks/          # GMock-based mocks
├── cmake/              # CMake toolchain files
├── docker/             # Dockerfile for build environment
├── appimage/           # AppImage packaging files
└── docs/               # Additional documentation
```

## Architecture

The application follows a layered architecture with Observer pattern for decoupling:

```
┌─────────────────────────────────────────────────────────────┐
│                      UI Layer (Qt/QML)                       │
│  EpisodeModel, CharacterModel, QmlBridge                    │
│                          │ implements IDataObserver          │
├──────────────────────────┼──────────────────────────────────┤
│                    Core Layer                                │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                   DataStore                          │    │
│  │  - Episodes/Characters/Locations cache              │    │
│  │  - Observer notifications                           │    │
│  └─────────────────────┬───────────────────────────────┘    │
│  ┌─────────────────────▼───────────────────────────────┐    │
│  │                   ApiClient                          │    │
│  │  - HTTP via IHttpClient (CURL implementation)       │    │
│  │  - JSON parsing via nlohmann/json                   │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| Qt6 | 6.8+ | UI framework (Quick, QML, QuickControls2) |
| libcurl | 8.x | HTTP client |
| nlohmann/json | 3.x | JSON parsing (header-only) |
| glog | 0.7+ | Logging |
| Google Test | 1.14+ | Testing framework |
| RapidCheck | latest | Property-based testing |

## API

The application consumes the [Rick and Morty API](https://rickandmortyapi.com/):

- **Base URL**: `https://rickandmortyapi.com/api`
- **Endpoints**: `/character`, `/episode`, `/location`
- **Authentication**: None required
- **Response Format**: JSON with pagination

See [docs/API.md](docs/API.md) for detailed API documentation.

## Documentation

- [Building Guide](docs/BUILDING.md) - Detailed build instructions
- [API Documentation](docs/API.md) - Rick and Morty API reference
- [Design Document](docs/DESIGN.md) - Architecture and design decisions
- [UI Design](docs/UI_DESIGN.md) - User interface specifications
- [Testing Guide](docs/TESTING.md) - Test infrastructure documentation

## CI/CD

GitHub Actions workflow builds and tests all supported platforms using two independent execution branches:

```
x86_64 Branch: build-linux-x86_64 → build-windows-x86_64
ARM64 Branch:  build-linux-arm64  → build-windows-arm64
```

| Platform | Build | Test Runner | Host Qt |
|----------|-------|-------------|---------|
| Linux x86_64 | Native x86_64 | Native Linux | Self |
| Windows x86_64 | MinGW cross-compile | Wine 8.0+ | linux-x86_64 |
| Linux ARM64 | QEMU emulation | QEMU ARM64 | Self |
| Windows ARM64 | LLVM-MinGW cross-compile | Wine 10.17 | linux-arm64 |

**Parallel execution:** x86_64 and ARM64 branches run independently. Windows builds start as soon as their corresponding Linux build completes.

**Wine 10 for ARM64:** Windows ARM64 tests use Wine 10.17 built from source.
Earlier Wine versions (8.0, 9.0) had ARM64 compatibility issues fixed in Wine 10.2+.

**Artifacts:**
- AppImage (Linux x86_64, Linux ARM64)
- Windows ZIP distributions (x86_64, ARM64)
- Prebuilt dependency tarballs (for faster CI rebuilds)
- Test result XML files (JUnit format)

## Troubleshooting

### Permission Issues (Docker)

Files created by Docker may be owned by root:

```bash
sudo chown -R $(whoami):$(whoami) .build/ .distribute/ .lib-prebuilt/
```

### Qt Build Failures

Common fixes:
- Install all `libxcb-*-dev` packages
- Install OpenGL development packages
- Ensure sufficient disk space (~5GB for Qt build)

### AppImage Won't Run

If FUSE is not available:

```bash
./RickAndMortyViewer-x86_64.AppImage --appimage-extract
./squashfs-root/AppRun
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Rick and Morty API](https://rickandmortyapi.com/) - Free API for Rick and Morty data
- [Qt Project](https://www.qt.io/) - Cross-platform application framework
- [Google Fonts](https://fonts.google.com/) - Bangers, Creepster, and Nunito fonts
