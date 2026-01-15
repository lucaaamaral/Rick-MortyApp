# Build environment for Rick and Morty Viewer
# Provides all dependencies needed to build Qt6 and the application
FROM debian:bookworm

ARG TARGETARCH

# Install build essentials and Qt dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build tools
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    perl \
    python3 \
    ca-certificates \
    curl \
    file \
    # Qt6 dependencies - X11/XCB
    libxcb1-dev \
    libxcb-cursor-dev \
    libxcb-glx0-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-shape0-dev \
    libxcb-shm0-dev \
    libxcb-sync-dev \
    libxcb-xfixes0-dev \
    libxcb-xinerama0-dev \
    libxcb-xkb-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libx11-dev \
    libx11-xcb-dev \
    libxext-dev \
    libxrender-dev \
    # OpenGL
    libgl1-mesa-dev \
    libegl1-mesa-dev \
    libgles2-mesa-dev \
    libdrm-dev \
    libgbm-dev \
    # Wayland (optional but good to have)
    libwayland-dev \
    libwayland-egl1-mesa \
    # Font rendering (critical for fontconfig support)
    libfontconfig1-dev \
    libfreetype-dev \
    # Image formats
    libpng-dev \
    libjpeg-dev \
    # SSL for curl
    libssl-dev \
    # Input
    libinput-dev \
    libmtdev-dev \
    libts-dev \
    # DBus (Qt may need it)
    libdbus-1-dev \
    # Compression
    zlib1g-dev \
    # AppImage tools
    fuse \
    libfuse2 \
    && rm -rf /var/lib/apt/lists/*

# Download AppImage tools
RUN ARCH=$(uname -m) && \
    # appimagetool for creating AppImages
    curl -L -o /usr/local/bin/appimagetool \
    "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-${ARCH}.AppImage" && \
    chmod +x /usr/local/bin/appimagetool && \
    # linuxdeploy for bundling dependencies
    curl -L -o /usr/local/bin/linuxdeploy \
    "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${ARCH}.AppImage" && \
    chmod +x /usr/local/bin/linuxdeploy && \
    # Qt plugin for linuxdeploy
    curl -L -o /usr/local/bin/linuxdeploy-plugin-qt \
    "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-${ARCH}.AppImage" && \
    chmod +x /usr/local/bin/linuxdeploy-plugin-qt

WORKDIR /build

# Default command runs cmake
CMD ["bash"]
