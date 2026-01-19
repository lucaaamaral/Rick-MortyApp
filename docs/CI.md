# CI/CD Workflow

This document describes the GitHub Actions CI/CD workflow architecture for building, testing, and releasing the Rick and Morty Viewer application.

## Overview

The CI workflow is defined in `.github/workflows/ci.yml` and supports:

- **4 target platforms**: Linux x86_64, Linux ARM64, Windows x86_64, Windows ARM64
- **Parallel execution**: x86_64 and ARM64 branches run independently
- **Smart caching**: Dependencies cached by content hash with release fallback
- **Wine testing**: Windows binaries tested via Wine on Linux

## Workflow Structure

The workflow uses two independent execution branches that run in parallel:

```
                    ┌─────────────────────────────────────────────────────────┐
                    │                     changes                              │
                    │              (detect code changes)                       │
                    └─────────────────────┬───────────────────────────────────┘
                                          │
              ┌───────────────────────────┴───────────────────────────┐
              │                                                       │
              ▼                                                       ▼
┌─────────────────────────────┐                     ┌─────────────────────────────┐
│     x86_64 BRANCH           │                     │      ARM64 BRANCH           │
├─────────────────────────────┤                     ├─────────────────────────────┤
│                             │                     │                             │
│  build-linux-x86_64         │                     │  build-linux-arm64          │
│  - Build Qt + deps          │                     │  - Build Qt + deps          │
│  - Build application        │                     │  - Build application        │
│  - Run Linux tests          │                     │  - Run Linux tests          │
│  - Create AppImage          │                     │  - Create AppImage          │
│  - Upload host Qt artifact  │                     │  - Upload host Qt artifact  │
│           │                 │                     │           │                 │
│           ▼                 │                     │           │ (parallel)      │
│  build-windows-x86_64       │                     │  build-wine10-arm64 ────┐   │
│  - Download host Qt         │                     │  - Build Wine 10        │   │
│  - Cross-compile app        │                     │  - Cache Docker image   │   │
│  - Run tests via Wine 8.0+  │                     │           │             │   │
│  - Create Windows ZIP       │                     │           ▼             │   │
│                             │                     │  build-windows-arm64 ◄──┘   │
│                             │                     │  - Download host Qt         │
│                             │                     │  - Download Wine 10 image   │
│                             │                     │  - Cross-compile app        │
│                             │                     │  - Run tests via Wine 10    │
│                             │                     │  - Create Windows ZIP       │
└─────────────────────────────┘                     └─────────────────────────────┘
```

## Jobs

### changes

Detects what files changed to determine if builds are needed:

- Runs on every push/PR
- Filters paths: `src/**`, `CMakeLists.txt`, `cmake/**`, `docker/**`, `.github/**`
- Skips builds if only documentation changed

### build-linux-x86_64 / build-linux-arm64

Builds Linux targets and produces host Qt for Windows cross-compilation:

1. **Restore cache** - Dependencies from previous runs
2. **Validate cache** - Check stamp files exist
3. **Fallback to prebuilt** - Download from GitHub release if cache invalid
4. **Build dependencies** - Qt6, curl, glog, fonts (if needed)
5. **Build application** - Main executable
6. **Build and run tests** - Unit tests with JUnit output
7. **Create AppImage** - Portable Linux distribution
8. **Upload artifacts** - Host Qt, AppImage, test results
9. **Save cache** - Dependencies for future runs

### build-wine10-arm64

Builds Wine 10 Docker image for ARM64 testing (runs in parallel with build-linux-arm64):

1. **Check cache** - Wine 10 image cached by Dockerfile hash
2. **Build Wine 10** - From source with ARM64 PE support (~8 min)
3. **Save artifact** - Docker image tarball for build-windows-arm64

### build-windows-x86_64 / build-windows-arm64

Cross-compiles Windows targets and tests via Wine:

1. **Download host Qt** - From corresponding Linux build
2. **Download Wine image** - (ARM64 only) From build-wine10-arm64
3. **Restore cache** - Windows dependencies
4. **Build dependencies** - Qt6 for Windows (cross-compile)
5. **Build application** - Windows executable
6. **Build and run tests** - Via Wine (8.0+ for x86_64, 10.17 for ARM64)
7. **Create ZIP** - Windows distribution package
8. **Upload artifacts** - ZIP, test results

## Caching Strategy

### Dependency Cache

Dependencies are cached using GitHub Actions cache with content-based keys:

```yaml
key: prebuilt-linux-x86_64-${{ github.ref_name }}-${{ hashFiles('.github/cache-inputs.txt') }}-v1
restore-keys: |
  prebuilt-linux-x86_64-master-${{ hashFiles('.github/cache-inputs.txt') }}-v1
  prebuilt-linux-x86_64-master-
  prebuilt-linux-x86_64-
```

**Cache key components:**
- `platform` - Target platform (linux-x86_64, windows-arm64, etc.)
- `branch` - Current branch name (feature branches inherit from master)
- `content hash` - Hash of `.github/cache-inputs.txt`
- `version` - Cache schema version (bump for incompatible changes)

### Cache Inputs

The `.github/cache-inputs.txt` file defines what invalidates the cache:

```
QT_VERSION=v6.6.2
CURL_VERSION=curl-8_5_0
GLOG_VERSION=v0.7.1
OPENSSL_VERSION=openssl-3.2.0
JSON_VERSION=v3.11.3
DEBIAN_VERSION=bullseye
CACHE_SCHEMA=1
```

### Cache Validation

After restoring cache, stamp files are validated:

```bash
for stamp in .qt-stamp .openssl-stamp .curl-stamp .glog-stamp .json-stamp; do
    if [[ ! -f ".lib-prebuilt/linux-x86_64/${stamp}" ]]; then
        echo "Cache invalid: missing ${stamp}"
    fi
done
```

### Prebuilt Fallback

If cache is invalid and not force-rebuilding, download prebuilt from releases:

```bash
gh release download v0.0.1 -p "lib-prebuilt-linux-x86_64.tar.xz"
tar -C .lib-prebuilt -xJf lib-prebuilt-linux-x86_64.tar.xz
```

### Wine 10 Caching

Wine 10 ARM64 image is cached as a GitHub Actions artifact:

- **Cache key**: Hash of `docker/Dockerfile.wine10-arm64`
- **First run**: Builds from source (~8 min on native ARM64)
- **Subsequent runs**: Loads cached image (seconds)

## Workflow Triggers

```yaml
on:
  push:
    branches: [main, master]
    tags: ['v*']
  pull_request:
    branches: [main, master]
  workflow_dispatch:
    inputs:
      force_rebuild_deps: boolean
      skip_x86_64: boolean
      skip_arm64: boolean
```

### Manual Dispatch Options

| Input | Description |
|-------|-------------|
| `force_rebuild_deps` | Ignore cache, rebuild all dependencies |
| `skip_x86_64` | Skip x86_64 branch (Linux + Windows) |
| `skip_arm64` | Skip ARM64 branch (Linux + Windows) |

## Artifacts

### Build Artifacts

| Artifact | Platform | Description |
|----------|----------|-------------|
| `RickAndMortyViewer-x86_64.AppImage` | Linux x86_64 | Portable Linux application |
| `RickAndMortyViewer-aarch64.AppImage` | Linux ARM64 | Portable Linux application |
| `RickAndMortyViewer-windows-x86_64.zip` | Windows x86_64 | Windows distribution |
| `RickAndMortyViewer-windows-arm64.zip` | Windows ARM64 | Windows distribution |

### Test Artifacts

| Artifact | Description |
|----------|-------------|
| `test-results-linux-x86_64.xml` | JUnit XML test results |
| `test-results-linux-arm64.xml` | JUnit XML test results |
| `test-results-windows-x86_64.xml` | JUnit XML test results |
| `test-results-windows-arm64.xml` | JUnit XML test results |

### Internal Artifacts

| Artifact | Description |
|----------|-------------|
| `host-qt-linux-x86_64` | Qt tools for Windows x86_64 cross-compile |
| `host-qt-linux-arm64` | Qt tools for Windows ARM64 cross-compile |
| `wine10-arm64-image` | Wine 10 Docker image for ARM64 testing |

## Runner Configuration

| Job | Runner | Notes |
|-----|--------|-------|
| changes | `ubuntu-latest` | Quick path filtering |
| build-linux-x86_64 | `ubuntu-latest` | Native x86_64 build |
| build-linux-arm64 | `ubuntu-22.04-arm` | Native ARM64 runner |
| build-wine10-arm64 | `ubuntu-22.04-arm` | Native ARM64 for Wine build |
| build-windows-x86_64 | `ubuntu-latest` | Cross-compile + Wine x86_64 |
| build-windows-arm64 | `ubuntu-22.04-arm` | Cross-compile + Wine 10 ARM64 |

**Note:** ARM64 jobs use native `ubuntu-22.04-arm` runners instead of QEMU emulation for ~10-20x faster builds.

## Concurrency

```yaml
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```

New pushes to the same branch cancel in-progress runs to save resources.

## See Also

- [Building Guide](BUILDING.md) - Local build instructions
- [Testing Guide](TESTING.md) - Test infrastructure
- [Docker Guide](DOCKER.md) - Docker build environment
