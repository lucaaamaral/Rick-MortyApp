#!/bin/bash
# Bundle glibc into AppImage for portability
# Usage: bundle-glibc.sh <appdir> <arch>
# arch: x86_64 or aarch64

set -e

APPDIR="$1"
ARCH="$2"

if [ -z "$APPDIR" ] || [ -z "$ARCH" ]; then
    echo "Usage: $0 <appdir> <arch>"
    exit 1
fi

GLIBC_DIR="${APPDIR}/usr/lib/glibc"
mkdir -p "$GLIBC_DIR"
APP_LIB_DIR="${APPDIR}/usr/lib"
mkdir -p "$APP_LIB_DIR"

echo "Bundling glibc for $ARCH into $GLIBC_DIR..."

if [ "$ARCH" = "x86_64" ]; then
    LIB_DIR="/usr/lib/x86_64-linux-gnu"
    LIB_DIR_ALT="/lib/x86_64-linux-gnu"
    LD_SO="/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2"
    LD_NAME="ld-linux-x86-64.so.2"
elif [ "$ARCH" = "aarch64" ]; then
    LIB_DIR="/usr/lib/aarch64-linux-gnu"
    LIB_DIR_ALT="/lib/aarch64-linux-gnu"
    LD_SO="/lib/aarch64-linux-gnu/ld-linux-aarch64.so.1"
    LD_NAME="ld-linux-aarch64.so.1"
else
    echo "Unknown architecture: $ARCH"
    exit 1
fi

# Check if glibc exists for this architecture
if [ ! -d "$LIB_DIR" ] && [ ! -d "$LIB_DIR_ALT" ]; then
    echo "Warning: $LIB_DIR and $LIB_DIR_ALT not found - glibc for $ARCH not available on this system"
    echo "AppImage will use system glibc at runtime"
    exit 0
fi

copy_lib_to() {
    local lib="$1"
    local dest="$2"
    if [ -f "$LIB_DIR/$lib" ]; then
        cp -L "$LIB_DIR/$lib" "$dest/" 2>/dev/null || true
        echo "Copied $lib"
    elif [ -f "$LIB_DIR_ALT/$lib" ]; then
        cp -L "$LIB_DIR_ALT/$lib" "$dest/" 2>/dev/null || true
        echo "Copied $lib"
    fi
}

# Copy dynamic linker
if [ -f "$LD_SO" ]; then
    cp -L "$LD_SO" "$GLIBC_DIR/$LD_NAME"
    echo "Copied $LD_SO"
else
    echo "Warning: Dynamic linker $LD_SO not found"
fi

# Copy core glibc libraries
GLIBC_LIBS="libc.so.6 libm.so.6 libpthread.so.0 libdl.so.2 librt.so.1"
GLIBC_LIBS="$GLIBC_LIBS libresolv.so.2 libnss_dns.so.2 libnss_files.so.2"
for lib in $GLIBC_LIBS; do
    copy_lib_to "$lib" "$GLIBC_DIR"
done

# Copy X11/xcb runtime dependencies for Qt's xcb plugin
X11_LIBS="libglib-2.0.so.0 libxcb.so.1 libX11.so.6 libXau.so.6 libXdmcp.so.6"
X11_LIBS="$X11_LIBS libxcb-shm.so.0 libxcb-render.so.0 libxcb-shape.so.0"
X11_LIBS="$X11_LIBS libxcb-xfixes.so.0 libxcb-sync.so.1 libxcb-randr.so.0"
X11_LIBS="$X11_LIBS libxcb-image.so.0 libxcb-keysyms.so.1 libxcb-icccm.so.4"
X11_LIBS="$X11_LIBS libxcb-render-util.so.0 libxcb-util.so.1 libxcb-cursor.so.0 libfontconfig.so.1"
for lib in $X11_LIBS; do
    copy_lib_to "$lib" "$APP_LIB_DIR"
done

# Copy OpenSSL runtime libraries for Qt's TLS backend
SSL_LIBS="libssl.so.1.1 libcrypto.so.1.1"
for lib in $SSL_LIBS; do
    copy_lib_to "$lib" "$APP_LIB_DIR"
done

# PCRE (Qt and other deps may use it)
PCRE_LIBS="libpcre.so.3"
for lib in $PCRE_LIBS; do
    copy_lib_to "$lib" "$APP_LIB_DIR"
done

# Copy libstdc++ and libgcc_s (C++ runtime)
for lib in libstdc++.so.6 libgcc_s.so.1; do
    if [ -f "$LIB_DIR/$lib" ]; then
        cp -L "$LIB_DIR/$lib" "$GLIBC_DIR/"
        echo "Copied $lib"
    elif [ -f "$LIB_DIR_ALT/$lib" ]; then
        cp -L "$LIB_DIR_ALT/$lib" "$GLIBC_DIR/"
        echo "Copied $lib"
    elif [ -f "/usr/lib/$lib" ]; then
        cp -L "/usr/lib/$lib" "$GLIBC_DIR/"
        echo "Copied $lib from /usr/lib"
    fi
done

echo "glibc bundling complete"
ls -la "$GLIBC_DIR/"
