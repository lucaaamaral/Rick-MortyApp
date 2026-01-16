# Build environment for Rick and Morty Viewer
# Provides all dependencies needed to build Qt6 and the application
FROM debian:bullseye

ARG TARGETARCH

# Install build essentials and Qt dependencies
RUN set -e; \
    ARCH="$(dpkg --print-architecture)"; \
    if [ "$ARCH" = "amd64" ]; then \
        FOREIGN_ARCH="arm64"; \
    elif [ "$ARCH" = "arm64" ]; then \
        FOREIGN_ARCH="amd64"; \
    else \
        echo "Unsupported architecture: $ARCH"; \
        exit 1; \
    fi; \
    dpkg --add-architecture "$FOREIGN_ARCH"; \
    if [ "$ARCH" = "amd64" ]; then \
        CROSS_COMPILERS="gcc-x86-64-linux-gnu g++-x86-64-linux-gnu gcc-aarch64-linux-gnu g++-aarch64-linux-gnu"; \
    else \
        CROSS_COMPILERS="gcc-x86-64-linux-gnu g++-x86-64-linux-gnu"; \
    fi; \
    apt-get update && apt-get install -y --no-install-recommends \
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
    xz-utils \
    # Linux cross-compilers (bidirectional arm64 <-> x86_64)
    ${CROSS_COMPILERS} \
    # Windows x86_64 cross-compiler (MinGW)
    mingw-w64 \
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
    libssl1.1 \
    libpcre3 \
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
    # Target sysroots (runtime libs for AppImage bundling)
    libc6-dev \
    libxcb1 \
    libxcb-shm0 \
    libxcb-render0 \
    libxcb-shape0 \
    libxcb-xfixes0 \
    libxcb-sync1 \
    libxcb-randr0 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-icccm4 \
    libxcb-render-util0 \
    libxcb-util1 \
    libxcb-xinerama0 \
    libxcb-xkb1 \
    libxcb-cursor0 \
    libx11-6 \
    libxau6 \
    libxdmcp6 \
    libglib2.0-0 \
    libfontconfig1 \
    libssl1.1 \
    libpcre3 \
    libc6-dev:${FOREIGN_ARCH} \
    libxcb1:${FOREIGN_ARCH} \
    libxcb-shm0:${FOREIGN_ARCH} \
    libxcb-render0:${FOREIGN_ARCH} \
    libxcb-shape0:${FOREIGN_ARCH} \
    libxcb-xfixes0:${FOREIGN_ARCH} \
    libxcb-sync1:${FOREIGN_ARCH} \
    libxcb-randr0:${FOREIGN_ARCH} \
    libxcb-image0:${FOREIGN_ARCH} \
    libxcb-keysyms1:${FOREIGN_ARCH} \
    libxcb-icccm4:${FOREIGN_ARCH} \
    libxcb-render-util0:${FOREIGN_ARCH} \
    libxcb-util1:${FOREIGN_ARCH} \
    libxcb-xinerama0:${FOREIGN_ARCH} \
    libxcb-xkb1:${FOREIGN_ARCH} \
    libxcb-cursor0:${FOREIGN_ARCH} \
    libx11-6:${FOREIGN_ARCH} \
    libxau6:${FOREIGN_ARCH} \
    libxdmcp6:${FOREIGN_ARCH} \
    libglib2.0-0:${FOREIGN_ARCH} \
    libfontconfig1:${FOREIGN_ARCH} \
    libssl1.1:${FOREIGN_ARCH} \
    libpcre3:${FOREIGN_ARCH} \
    && rm -rf /var/lib/apt/lists/*; \
    if [ "$ARCH" = "arm64" ]; then \
        ln -sf /usr/bin/gcc /usr/bin/aarch64-linux-gnu-gcc; \
        ln -sf /usr/bin/g++ /usr/bin/aarch64-linux-gnu-g++; \
    fi

# Install a newer CMake than bullseye provides (required for Qt 6.6+)
ARG CMAKE_VERSION=3.27.9
RUN set -e; \
    ARCH="$(uname -m)"; \
    if [ "$ARCH" = "x86_64" ]; then CMAKE_ARCH="x86_64"; \
    elif [ "$ARCH" = "aarch64" ]; then CMAKE_ARCH="aarch64"; \
    else echo "Unsupported CMake arch: $ARCH"; exit 1; fi; \
    curl -L -o /tmp/cmake.tar.gz \
        "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-${CMAKE_ARCH}.tar.gz"; \
    tar -xzf /tmp/cmake.tar.gz -C /opt; \
    ln -sf /opt/cmake-${CMAKE_VERSION}-linux-${CMAKE_ARCH}/bin/* /usr/local/bin/; \
    rm /tmp/cmake.tar.gz

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

# Install LLVM-MinGW for Windows ARM64 cross-compilation
# Downloads the correct version based on host architecture
RUN ARCH=$(uname -m) && \
    if [ "$ARCH" = "x86_64" ]; then LLVM_ARCH="x86_64"; else LLVM_ARCH="aarch64"; fi && \
    curl -L -o /tmp/llvm-mingw.tar.xz \
    "https://github.com/mstorsjo/llvm-mingw/releases/download/20251216/llvm-mingw-20251216-ucrt-ubuntu-22.04-${LLVM_ARCH}.tar.xz" && \
    tar -xf /tmp/llvm-mingw.tar.xz -C /opt && \
    ln -s /opt/llvm-mingw-20251216-ucrt-ubuntu-22.04-* /opt/llvm-mingw && \
    rm /tmp/llvm-mingw.tar.xz

ENV PATH="/opt/llvm-mingw/bin:${PATH}"

WORKDIR /build

# Default command runs cmake
CMD ["bash"]
