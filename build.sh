#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# Detect host architecture
detect_arch() {
    case "$(uname -m)" in
        x86_64|amd64) echo "amd64" ;;
        aarch64|arm64) echo "arm64" ;;
        *) echo "amd64" ;;  # Default to amd64
    esac
}

# Map docker arch to platform name
arch_to_platform() {
    case "$1" in
        amd64) echo "linux-x86_64" ;;
        arm64) echo "linux-arm64" ;;
        *) echo "linux-x86_64" ;;
    esac
}

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}======================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}======================================${NC}"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Build Docker image
build_image() {
    local arch="$1"
    print_header "Building Docker image for $arch"
    docker compose build "deps-$arch"
}

# Build dependencies (Qt + curl + glog + fonts)
build_deps() {
    local arch="${1:-$(detect_arch)}"
    local platform=$(arch_to_platform "$arch")

    print_header "Building dependencies for $platform"
    echo "This may take 2-3 hours on first run..."

    # Check if deps already exist
    if [[ -f ".lib-prebuilt/$platform/.qt-stamp" ]]; then
        print_warning "Dependencies already built. Use 'clean' to rebuild."
        return 0
    fi

    docker compose run --rm "deps-$arch"
    print_success "Dependencies built successfully!"
}

# Build application
build_app() {
    local arch="${1:-$(detect_arch)}"
    local platform=$(arch_to_platform "$arch")

    print_header "Building application for $platform"

    # Check if deps exist
    if [[ ! -f ".lib-prebuilt/$platform/.qt-stamp" ]]; then
        print_error "Dependencies not built. Run './build.sh deps $arch' first."
        exit 1
    fi

    docker compose run --rm "app-$arch"
    print_success "Application built successfully!"
}

# Create distribution
build_distribute() {
    local arch="${1:-$(detect_arch)}"
    local platform=$(arch_to_platform "$arch")

    print_header "Creating distribution for $platform"
    docker compose run --rm "distribute-$arch"
    print_success "Distribution created in .distribute/$platform/"
}

# Create AppImage
build_appimage() {
    local arch="${1:-$(detect_arch)}"
    local platform=$(arch_to_platform "$arch")

    print_header "Creating AppImage for $platform"
    docker compose run --rm "appimage-$arch"
    print_success "AppImage created!"
}

# Build everything
build_all() {
    local arch="${1:-$(detect_arch)}"
    build_deps "$arch"
    build_app "$arch"
}

# Full release build
build_release() {
    local arch="${1:-$(detect_arch)}"
    build_all "$arch"
    build_distribute "$arch"
    build_appimage "$arch"
}

# Clean build artifacts
clean() {
    local arch="${1:-}"

    if [[ -n "$arch" ]]; then
        local platform=$(arch_to_platform "$arch")
        print_header "Cleaning $platform build artifacts"
        rm -rf ".build/$platform"
        rm -rf ".distribute/$platform"
        print_success "Cleaned $platform artifacts"
    else
        print_header "Cleaning all build artifacts"
        rm -rf .build
        rm -rf .distribute
        print_success "Cleaned all artifacts"
    fi
}

# Clean everything including prebuilt deps
clean_all() {
    print_header "Cleaning everything (including prebuilt deps)"
    rm -rf .build
    rm -rf .distribute
    rm -rf .lib-prebuilt
    print_success "Cleaned everything"
}

# Show status
status() {
    print_header "Build Status"

    echo ""
    echo "Prebuilt dependencies:"
    for platform in linux-x86_64 linux-arm64; do
        if [[ -f ".lib-prebuilt/$platform/.qt-stamp" ]]; then
            echo -e "  ${GREEN}[OK]${NC} $platform"
            if [[ -d ".lib-prebuilt/$platform/qt6" ]]; then
                local qt_size=$(du -sh ".lib-prebuilt/$platform/qt6" 2>/dev/null | cut -f1)
                echo "       Qt6: $qt_size"
            fi
        else
            echo -e "  ${RED}[--]${NC} $platform (not built)"
        fi
    done

    echo ""
    echo "Build cache:"
    for platform in linux-x86_64 linux-arm64; do
        if [[ -d ".build/$platform" ]]; then
            local size=$(du -sh ".build/$platform" 2>/dev/null | cut -f1)
            echo -e "  ${GREEN}[OK]${NC} $platform ($size)"
        else
            echo -e "  ${RED}[--]${NC} $platform"
        fi
    done

    echo ""
    echo "Distribution:"
    for platform in linux-x86_64 linux-arm64; do
        if [[ -f ".distribute/$platform/RickAndMortyViewer" ]]; then
            echo -e "  ${GREEN}[OK]${NC} $platform"
            if [[ -f ".distribute/RickAndMortyViewer-${platform#linux-}.AppImage" ]]; then
                echo -e "       ${GREEN}[OK]${NC} AppImage"
            fi
        else
            echo -e "  ${RED}[--]${NC} $platform"
        fi
    done
}

# Print usage
usage() {
    echo "Usage: $0 <command> [arch]"
    echo ""
    echo "Commands:"
    echo "  deps [arch]       Build Qt + dependencies (~2-3 hours first time)"
    echo "  app [arch]        Build application (~5 min)"
    echo "  all [arch]        Build deps + app"
    echo "  distribute [arch] Create lib/ directory distribution"
    echo "  appimage [arch]   Create AppImage from distribution"
    echo "  release [arch]    Full build: deps + app + distribute + appimage"
    echo "  clean [arch]      Clean build cache (keeps prebuilt deps)"
    echo "  clean-all         Clean everything including prebuilt deps"
    echo "  status            Show build status"
    echo ""
    echo "Architecture:"
    echo "  amd64             x86_64 (default on x86 hosts)"
    echo "  arm64             aarch64 (default on ARM hosts)"
    echo ""
    echo "Examples:"
    echo "  $0 deps           # Build deps for current arch"
    echo "  $0 app arm64      # Build app for ARM64"
    echo "  $0 release        # Full release for current arch"
    echo "  $0 status         # Show what's built"
}

# Main
case "${1:-}" in
    deps)       build_deps "${2:-}" ;;
    app)        build_app "${2:-}" ;;
    all)        build_all "${2:-}" ;;
    distribute) build_distribute "${2:-}" ;;
    appimage)   build_appimage "${2:-}" ;;
    release)    build_release "${2:-}" ;;
    clean)      clean "${2:-}" ;;
    clean-all)  clean_all ;;
    status)     status ;;
    -h|--help|help|"")  usage ;;
    *)
        print_error "Unknown command: $1"
        usage
        exit 1
        ;;
esac
