# Docker Build Guide

This document provides detailed instructions for building the Rick and Morty Viewer application using Docker.

## Overview

Docker provides a consistent, reproducible build environment with all dependencies pre-configured. This is the recommended approach for building the application, especially for cross-compilation.

## Prerequisites

### Install Docker

**Ubuntu/Debian:**
```bash
# Install Docker
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh

# Add user to docker group (logout/login required)
sudo usermod -aG docker $USER

# Install Docker Compose plugin
sudo apt install docker-compose-plugin
```

**Verify installation:**
```bash
docker --version
docker compose version
```

## Docker Architecture

### Dockerfile

The build environment is defined in `docker/Dockerfile`:

- **Base image**: Debian Bullseye
- **Build tools**: CMake, Ninja, GCC/G++
- **Cross-compilers**:
  - `aarch64-linux-gnu-gcc` (Linux ARM64)
  - `x86_64-linux-gnu-gcc` (Linux x86_64)
  - MinGW (Windows x86_64)
  - LLVM-MinGW (Windows ARM64)
- **Qt dependencies**: XCB, OpenGL, Wayland libraries
- **AppImage tools**: appimagetool, linuxdeploy

### Docker Compose Services

The `docker-compose.yml` defines services for each build step and platform:

**Build Services:**

| Service | Purpose |
|---------|---------|
| `deps-linux-x86_64` / `deps-linux-arm64` | Build Qt6 + curl + glog + fonts for Linux |
| `deps-windows-x86_64` / `deps-windows-arm64` | Build Qt6 + curl + glog for Windows (cross-compile) |
| `app-linux-x86_64` / `app-linux-arm64` | Build Linux application |
| `app-windows-x86_64` / `app-windows-arm64` | Build Windows application (cross-compile) |

**Distribution Services:**

| Service | Purpose |
|---------|---------|
| `distribute-linux-x86_64` / `distribute-linux-arm64` | Create Linux distribution bundle |
| `distribute-windows-x86_64` / `distribute-windows-arm64` | Create Windows distribution bundle |
| `appimage-linux-x86_64` / `appimage-linux-arm64` | Create Linux AppImage |
| `package-windows-x86_64` / `package-windows-arm64` | Create Windows ZIP distribution |
| `bundle-glibc-linux-x86_64` | Bundle glibc for portable Linux builds |

**Testing Services:**

| Service | Purpose | Runner |
|---------|---------|--------|
| `test-build-linux-x86_64` / `test-build-linux-arm64` | Build Linux test executables | Native |
| `test-build-windows-x86_64` / `test-build-windows-arm64` | Build Windows test executables | Native |
| `test-run-linux-x86_64` / `test-run-linux-arm64` | Run Linux tests | Native Linux |
| `test-run-windows-x86_64` | Run Windows x86_64 tests | Wine 8.0+ |
| `test-run-windows-arm64` | Run Windows ARM64 tests | Wine 10.17 |
| `smoke-test-linux-x86_64` / `smoke-test-linux-arm64` | Quick smoke test | Native |

**Utility Services:**

| Service | Purpose |
|---------|---------|
| `clean-linux-x86_64` / `clean-linux-arm64` | Remove Linux build artifacts |
| `clean-windows-x86_64` / `clean-windows-arm64` | Remove Windows build artifacts |
| `prebuilt-linux-x86_64` / `prebuilt-linux-arm64` | Create prebuilt dependency tarballs |
| `prebuilt-windows-x86_64` / `prebuilt-windows-arm64` | Create prebuilt dependency tarballs |

## Using build.sh

The `build.sh` script provides a convenient wrapper around Docker Compose:

```bash
# Show all commands
./build.sh --help

# Build for current architecture
./build.sh deps      # Build dependencies (~2-3 hours)
./build.sh app       # Build application (~1-2 min)
./build.sh appimage  # Create AppImage

# Build for specific architecture
./build.sh deps arm64     # ARM64 dependencies
./build.sh app arm64      # ARM64 application
./build.sh appimage arm64 # ARM64 AppImage

# Full release build
./build.sh release        # deps + app + distribute + appimage

# Check build status
./build.sh status

# Clean builds
./build.sh clean          # Remove build artifacts (keep deps)
./build.sh clean-all      # Remove everything including deps
```

## Manual Docker Compose Usage

### Building Dependencies

Dependencies (Qt6, curl, glog, fonts) are built once and cached:

```bash
# Native build (matches host architecture)
docker compose run --rm deps-amd64   # On x86_64 host
docker compose run --rm deps-arm64   # On ARM64 host

# Cross-compilation
docker compose run --rm deps-linux-x86_64   # x86_64 from any host
docker compose run --rm deps-linux-arm64    # ARM64 from any host
docker compose run --rm deps-windows-x86_64 # Windows x86_64
docker compose run --rm deps-windows-arm64  # Windows ARM64
```

**First-time build takes 2-3 hours** for Qt compilation. Subsequent builds use cached dependencies.

### Building the Application

```bash
# Native build
docker compose run --rm app-amd64
docker compose run --rm app-arm64

# Cross-compilation
docker compose run --rm app-linux-x86_64
docker compose run --rm app-linux-arm64
docker compose run --rm app-windows-x86_64
docker compose run --rm app-windows-arm64
```

### Creating Distribution

```bash
# Create distribution bundle
docker compose run --rm distribute-amd64
docker compose run --rm distribute-arm64

# Create AppImage (Linux only)
docker compose run --rm appimage-amd64
docker compose run --rm appimage-arm64
```

### Running Tests

```bash
# Build test executables
docker compose run --rm test-build-linux-x86_64
docker compose run --rm test-build-windows-x86_64

# Run tests (Linux - native)
docker compose run --rm test-run-linux-x86_64
docker compose run --rm test-run-linux-arm64

# Run tests (Windows - via Wine)
docker compose run --rm test-run-windows-x86_64   # Uses Wine 8.0+
docker compose run --rm test-run-windows-arm64    # Uses Wine 10.17

# Quick smoke test
docker compose run --rm smoke-test-linux-x86_64
```

### Wine Testing

Windows executables are tested via Wine on Linux:

| Platform | Wine Version | Docker Image |
|----------|--------------|--------------|
| Windows x86_64 | Wine 8.0+ (system) | `Dockerfile.wine64` |
| Windows ARM64 | Wine 10.17 (built from source) | `Dockerfile.wine10-arm64` |

**Wine 10 for ARM64:**
Wine 8.0/9.0 had bugs causing ARM64 Wine to hang or crash during initialization.
Wine 10.2+ fixes this ([LP#1100695](https://bugs.launchpad.net/wine/+bug/1100695)).

The `Dockerfile.wine10-arm64` builds Wine 10.17 from source with ARM64 PE support.
First build takes ~8 minutes on native ARM64, but is cached for subsequent runs.

## Volume Mounts

Docker containers mount the project directory at `/workspace`:

```yaml
volumes:
  - .:/workspace:rw
```

Build artifacts are written to:
- `.build/` - CMake build directories
- `.lib-prebuilt/` - Cached dependencies
- `.distribute/` - Distribution bundles

## Caching Strategy

### Dependency Caching

Dependencies are cached in `.lib-prebuilt/<platform>/`:

```
.lib-prebuilt/
├── linux-x86_64/
│   ├── qt6/           # Qt installation (~1.5GB)
│   ├── curl/          # libcurl installation
│   ├── glog/          # glog installation
│   ├── fonts/         # Font files
│   ├── .qt-stamp      # Qt build marker
│   ├── .curl-stamp    # curl build marker
│   └── .glog-stamp    # glog build marker
├── linux-arm64/
│   └── ...
├── windows-x86_64/
│   └── ...
└── windows-arm64/
    └── ...
```

### Force Rebuild

To force rebuilding a specific dependency:

```bash
# Force Qt rebuild
rm .lib-prebuilt/linux-x86_64/.qt-stamp
docker compose run --rm deps-linux-x86_64

# Force all dependencies rebuild
rm -rf .lib-prebuilt/linux-x86_64
docker compose run --rm deps-linux-x86_64
```

### Clean Services

Use clean services to reset build state:

```bash
docker compose run --rm clean-linux-x86_64
docker compose run --rm clean-linux-arm64
docker compose run --rm clean-windows-x86_64
docker compose run --rm clean-windows-arm64
```

## Multi-Platform Builds

### Building All Platforms

```bash
# Build dependencies for all platforms (can run in parallel)
docker compose run --rm deps-linux-x86_64 &
docker compose run --rm deps-linux-arm64 &
docker compose run --rm deps-windows-x86_64 &
docker compose run --rm deps-windows-arm64 &
wait

# Build applications
docker compose run --rm app-linux-x86_64
docker compose run --rm app-linux-arm64
docker compose run --rm app-windows-x86_64
docker compose run --rm app-windows-arm64

# Create distributions
docker compose run --rm appimage-linux-x86_64
docker compose run --rm appimage-linux-arm64
```

### Cross-Architecture Docker

For cross-architecture builds (e.g., building ARM64 on x86_64):

```bash
# Enable QEMU for cross-architecture containers
docker run --privileged --rm tonistiigi/binfmt --install all

# Now ARM64 containers can run on x86_64
docker compose run --rm deps-arm64
```

## Troubleshooting

### Permission Issues

Docker creates files as root. Fix ownership:

```bash
sudo chown -R $(whoami):$(whoami) .build/ .distribute/ .lib-prebuilt/
```

Or run Docker with user mapping:

```bash
docker compose run --rm --user $(id -u):$(id -g) app-amd64
```

### Disk Space

A full build requires significant disk space:

| Component | Size |
|-----------|------|
| Docker image | ~2GB |
| Qt build | ~5GB (during build) |
| Qt installation | ~1.5GB |
| Application build | ~500MB |
| Total for one platform | ~8GB |
| All 4 platforms | ~30GB |

Clean up after build:

```bash
# Remove Qt build artifacts (keep installation)
rm -rf .lib-prebuilt/*/qt6-build
rm -rf .lib-prebuilt/*/qt6-src

# Remove Docker build cache
docker builder prune
```

### Container Won't Start

Check Docker daemon:

```bash
sudo systemctl status docker
sudo systemctl start docker
```

### Build Fails with "No space left"

Docker storage may be full:

```bash
# Check Docker disk usage
docker system df

# Clean unused images and containers
docker system prune -a

# Clean build cache
docker builder prune -a
```

### AppImage Tools Fail

AppImage tools require FUSE inside the container. The Dockerfile installs required packages, but if issues persist:

```bash
# Run container with FUSE access
docker compose run --rm --privileged appimage-amd64
```

### Network Issues During Build

If downloads fail, check DNS and proxy settings:

```bash
# Test connectivity inside container
docker compose run --rm deps-amd64 bash -c "curl -I https://github.com"

# Use host network if needed
docker compose run --rm --network host deps-amd64
```

## Performance Tips

### Parallel Builds

Enable parallel make jobs (default is auto-detected):

```bash
# Override in docker-compose.yml or command
docker compose run --rm app-amd64 bash -c "cmake --build .build/linux-x86_64 -j$(nproc)"
```

### BuildKit

Enable Docker BuildKit for faster builds:

```bash
export DOCKER_BUILDKIT=1
export COMPOSE_DOCKER_CLI_BUILD=1
docker compose build
```

### Caching Across CI Runs

For CI/CD, cache `.lib-prebuilt/`:

```yaml
# GitHub Actions example
- uses: actions/cache@v4
  with:
    path: |
      .lib-prebuilt/linux-x86_64/qt6
      .lib-prebuilt/linux-x86_64/curl
      .lib-prebuilt/linux-x86_64/glog
    key: deps-linux-x86_64-${{ hashFiles('CMakeLists.txt') }}
```

## See Also

- [Building Guide](BUILDING.md) - General build instructions
- [Docker Documentation](https://docs.docker.com/)
- [Docker Compose Documentation](https://docs.docker.com/compose/)
