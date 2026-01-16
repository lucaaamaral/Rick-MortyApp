cmake_minimum_required(VERSION 3.16)
project(RickAndMortyViewerSuperbuild)

include(ExternalProject)

#######################################
# Platform Detection / Selection
#######################################
if(NOT DEFINED TARGET_PLATFORM)
    # Auto-detect from host
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            set(TARGET_PLATFORM "linux-arm64")
        else()
            set(TARGET_PLATFORM "linux-x86_64")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64")
            set(TARGET_PLATFORM "windows-arm64")
        else()
            set(TARGET_PLATFORM "windows-x86_64")
        endif()
    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
    endif()
endif()

# Validate TARGET_PLATFORM
set(VALID_PLATFORMS "linux-x86_64;linux-arm64;windows-x86_64;windows-arm64")
if(NOT TARGET_PLATFORM IN_LIST VALID_PLATFORMS)
    message(FATAL_ERROR "Invalid TARGET_PLATFORM: ${TARGET_PLATFORM}\nValid options: ${VALID_PLATFORMS}")
endif()

# Parse platform components
string(REPLACE "-" ";" PLATFORM_PARTS ${TARGET_PLATFORM})
list(GET PLATFORM_PARTS 0 TARGET_OS)
list(GET PLATFORM_PARTS 1 TARGET_ARCH)

# Determine native host platform (always use host, not target/toolchain)
set(HOST_SYSTEM_NAME "${CMAKE_HOST_SYSTEM_NAME}")
if(NOT HOST_SYSTEM_NAME)
    set(HOST_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
endif()
set(HOST_SYSTEM_PROCESSOR "${CMAKE_HOST_SYSTEM_PROCESSOR}")
if(NOT HOST_SYSTEM_PROCESSOR OR HOST_SYSTEM_PROCESSOR STREQUAL "unknown")
    set(HOST_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
endif()
if(NOT HOST_SYSTEM_PROCESSOR OR HOST_SYSTEM_PROCESSOR STREQUAL "unknown")
    if(HOST_SYSTEM_NAME STREQUAL "Linux")
        execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE HOST_SYSTEM_PROCESSOR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
endif()

if(HOST_SYSTEM_NAME STREQUAL "Linux")
    if(HOST_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(HOST_PLATFORM "linux-arm64")
    else()
        set(HOST_PLATFORM "linux-x86_64")
    endif()
else()
    message(WARNING "Unknown host OS: ${HOST_SYSTEM_NAME}; defaulting HOST_PLATFORM to linux-x86_64")
    set(HOST_PLATFORM "linux-x86_64")
endif()

# Determine if cross-compiling
if(HOST_SYSTEM_NAME STREQUAL "Linux" AND TARGET_OS STREQUAL "linux")
    # Check if target arch matches host arch
    if(TARGET_ARCH STREQUAL "x86_64" AND HOST_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(IS_CROSS_COMPILE FALSE)
    elseif(TARGET_ARCH MATCHES "arm64|aarch64" AND HOST_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(IS_CROSS_COMPILE FALSE)
    else()
        set(IS_CROSS_COMPILE TRUE)
    endif()
else()
    set(IS_CROSS_COMPILE TRUE)
endif()

message(STATUS "========================================")
message(STATUS "Host Platform: ${HOST_PLATFORM}")
message(STATUS "Host CPU: ${HOST_SYSTEM_PROCESSOR}")
message(STATUS "Target Platform: ${TARGET_PLATFORM}")
message(STATUS "Target OS: ${TARGET_OS}")
message(STATUS "Target Arch: ${TARGET_ARCH}")
message(STATUS "Cross-compiling: ${IS_CROSS_COMPILE}")
message(STATUS "========================================")

#######################################
# Directories
#######################################
set(PREBUILT_DIR "${CMAKE_SOURCE_DIR}/.lib-prebuilt/${TARGET_PLATFORM}")
set(QT_INSTALL_DIR "${PREBUILT_DIR}/qt6")
set(QT_STAMP_FILE "${PREBUILT_DIR}/.qt-stamp")
set(JSON_INSTALL_DIR "${PREBUILT_DIR}/json")
set(JSON_STAMP_FILE "${PREBUILT_DIR}/.json-stamp")
set(CURL_INSTALL_DIR "${PREBUILT_DIR}/curl")
set(CURL_STAMP_FILE "${PREBUILT_DIR}/.curl-stamp")
set(GLOG_INSTALL_DIR "${PREBUILT_DIR}/glog")
set(GLOG_STAMP_FILE "${PREBUILT_DIR}/.glog-stamp")
set(DIST_DIR "${CMAKE_SOURCE_DIR}/.distribute/${TARGET_PLATFORM}")

# Number of parallel jobs
include(ProcessorCount)
ProcessorCount(NPROC)
if(NPROC EQUAL 0)
    set(NPROC 4)
endif()

message(STATUS "Prebuilt directory: ${PREBUILT_DIR}")
message(STATUS "Parallel jobs: ${NPROC}")

#######################################
# Platform-specific Qt Configuration
#######################################
set(QT_GENERATOR "Unix Makefiles")

if(TARGET_OS STREQUAL "linux")
    set(LIB_EXTENSION "so")
    set(EXE_EXTENSION "")
    # Native build: use system libraries
    # Cross-compile: use bundled libraries (no sysroot available)
    if(IS_CROSS_COMPILE)
        set(QT_PLATFORM_ARGS
            -no-fontconfig
            -no-opengl
            -no-feature-brotli
            -no-feature-mtdev
            -no-feature-tslib
            -no-feature-libinput
            -no-feature-evdev
            -no-feature-linuxfb
            -no-feature-vnc
            -qt-freetype
            -qt-pcre
            -qt-libpng
            -qt-libjpeg
            -qt-zlib
            -qt-harfbuzz
        )
    else()
        # Native build: Let Qt auto-detect available platform backends
        set(QT_PLATFORM_ARGS
            -fontconfig
        )
    endif()
elseif(TARGET_OS STREQUAL "windows")
    set(QT_PLATFORM_ARGS
        -platform win32-g++
        -no-opengl
    )
    set(LIB_EXTENSION "dll")
    set(EXE_EXTENSION ".exe")
endif()

# Cross-compilation setup
if(IS_CROSS_COMPILE)
    # For cross-compiling Qt, we need a host Qt first (for moc, rcc, qmlcachegen)
    # Qt6 puts host tools in libexec/, not bin/
    set(HOST_QT_DIR "${CMAKE_SOURCE_DIR}/.lib-prebuilt/${HOST_PLATFORM}/qt6")
    if(NOT EXISTS "${HOST_QT_DIR}/libexec/moc")
        message(FATAL_ERROR
            "Cross-compiling requires host Qt built first.\n"
            "Build for ${HOST_PLATFORM} before building for ${TARGET_PLATFORM}:\n"
            "  cmake -B .build/${HOST_PLATFORM} -DTARGET_PLATFORM=${HOST_PLATFORM}\n"
            "  cmake --build .build/${HOST_PLATFORM} --target qt6_external"
        )
    endif()
    message(STATUS "Using host Qt from: ${HOST_QT_DIR}")

    # Set toolchain file path
    set(CROSS_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/toolchains/${TARGET_PLATFORM}.cmake")
endif()

#######################################
# Qt6 from source (if not already built)
# Requires system packages: libwayland-dev libxcb1-dev libxcb-*-dev libxkbcommon-x11-dev libfontconfig1-dev
#######################################
if(NOT EXISTS "${QT_STAMP_FILE}")
    message(STATUS "Qt6 not found in ${QT_INSTALL_DIR}")
    message(STATUS "Will build Qt6 from source (this takes 30-60 minutes first time)")

    # Create prebuilt directory
    file(MAKE_DIRECTORY ${PREBUILT_DIR})

    # Build Qt configure command
    set(QT_CONFIGURE_CMD
        <SOURCE_DIR>/configure
        -prefix ${QT_INSTALL_DIR}
        -release
        -shared
        -opensource
        -confirm-license
        -nomake examples
        -nomake tests
        -nomake benchmarks
        -no-feature-testlib
        -no-feature-sql
        -no-feature-dbus
        -no-feature-printsupport
        -no-feature-cups
        -no-feature-concurrent
        ${QT_PLATFORM_ARGS}
    )

    if(IS_CROSS_COMPILE AND TARGET_OS STREQUAL "linux")
        set(SYSROOT_DIR "/")
        if(TARGET_PLATFORM STREQUAL "linux-arm64")
            set(PKG_CONFIG_PATH "/usr/lib/aarch64-linux-gnu/pkgconfig")
        elseif(TARGET_PLATFORM STREQUAL "linux-x86_64")
            set(PKG_CONFIG_PATH "/usr/lib/x86_64-linux-gnu/pkgconfig")
        endif()

        list(APPEND QT_CONFIGURE_CMD
            -sysroot ${SYSROOT_DIR}
            -pkg-config
        )
        list(PREPEND QT_CONFIGURE_CMD
            ${CMAKE_COMMAND} -E env
            PKG_CONFIG_PATH=${PKG_CONFIG_PATH}
            PKG_CONFIG_LIBDIR=${PKG_CONFIG_PATH}
            PKG_CONFIG_SYSROOT_DIR=${SYSROOT_DIR}
        )
    endif()

    # Add cross-compile options if needed
    # Qt6 uses CMake internally - pass toolchain file via -- options
    if(IS_CROSS_COMPILE)
        # Host Qt path for moc, rcc, qmlcachegen
        list(APPEND QT_CONFIGURE_CMD -qt-host-path ${HOST_QT_DIR})

        # Pass CMake options after --
        list(APPEND QT_CONFIGURE_CMD
            --
            -G "${QT_GENERATOR}"
            -DCMAKE_TOOLCHAIN_FILE=${CROSS_TOOLCHAIN_FILE}
            -DCMAKE_C_COMPILER_WORKS=TRUE
            -DCMAKE_CXX_COMPILER_WORKS=TRUE
        )
    else()
        list(APPEND QT_CONFIGURE_CMD -- -G "${QT_GENERATOR}")
    endif()

    ExternalProject_Add(qt6_external
        GIT_REPOSITORY https://code.qt.io/qt/qt5.git
        GIT_TAG v6.6.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        GIT_SUBMODULES ""

        PREFIX ${PREBUILT_DIR}/qt6-build
        SOURCE_DIR ${PREBUILT_DIR}/qt6-src
        BINARY_DIR ${PREBUILT_DIR}/qt6-build
        INSTALL_DIR ${QT_INSTALL_DIR}

        # Initialize only needed submodules with shallow clone (much faster)
        # Includes qtwayland for Wayland display support
        UPDATE_COMMAND cd <SOURCE_DIR> &&
            git submodule update --init --depth 1 --single-branch -- qtbase qtdeclarative qtshadertools qtsvg qtwayland

        CONFIGURE_COMMAND ${QT_CONFIGURE_CMD}

        BUILD_COMMAND cmake --build <BINARY_DIR> --parallel ${NPROC}
        INSTALL_COMMAND cmake --install <BINARY_DIR>

        # Create stamp file after successful install
        COMMAND ${CMAKE_COMMAND} -E touch ${QT_STAMP_FILE}

        # Show output in terminal
        USES_TERMINAL_DOWNLOAD TRUE
        USES_TERMINAL_UPDATE TRUE
        USES_TERMINAL_CONFIGURE TRUE
        USES_TERMINAL_BUILD TRUE
        USES_TERMINAL_INSTALL TRUE
    )

    set(QT_DEPENDENCY qt6_external)
else()
    message(STATUS "Using cached Qt6 from ${QT_INSTALL_DIR}")
    set(QT_DEPENDENCY "")
endif()

#######################################
# Fonts (for portable distribution)
# Rick and Morty themed font hierarchy:
# - Bangers: Comic book style for titles
# - Creepster: Horror style for episode codes
# - Nunito: Rounded body text
# - Noto Color Emoji for emoji support
#
# Caching: Each font is checked individually.
# Only missing fonts are downloaded.
#######################################
set(FONTS_DIR "${PREBUILT_DIR}/fonts")
set(FONTS_DEPENDENCY "")

# Ensure fonts directory exists
file(MAKE_DIRECTORY ${FONTS_DIR})

# Bangers - Comic book display font for main titles
if(NOT EXISTS "${FONTS_DIR}/Bangers-Regular.ttf")
    message(STATUS "Downloading Bangers font...")
    ExternalProject_Add(bangers_font
        URL https://raw.githubusercontent.com/google/fonts/main/ofl/bangers/Bangers-Regular.ttf
        DOWNLOAD_NO_EXTRACT TRUE
        DOWNLOAD_NAME Bangers-Regular.ttf
        PREFIX ${PREBUILT_DIR}/fonts-build/bangers
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
            ${PREBUILT_DIR}/fonts-build/bangers/src/Bangers-Regular.ttf
            ${FONTS_DIR}/
    )
    set(FONTS_DEPENDENCY bangers_font)
else()
    message(STATUS "Using cached Bangers font")
endif()

# Creepster - Horror display font for episode codes
if(NOT EXISTS "${FONTS_DIR}/Creepster-Regular.ttf")
    message(STATUS "Downloading Creepster font...")
    ExternalProject_Add(creepster_font
        URL https://raw.githubusercontent.com/google/fonts/main/ofl/creepster/Creepster-Regular.ttf
        DOWNLOAD_NO_EXTRACT TRUE
        DOWNLOAD_NAME Creepster-Regular.ttf
        PREFIX ${PREBUILT_DIR}/fonts-build/creepster
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
            ${PREBUILT_DIR}/fonts-build/creepster/src/Creepster-Regular.ttf
            ${FONTS_DIR}/
    )
    if(FONTS_DEPENDENCY)
        add_dependencies(creepster_font ${FONTS_DEPENDENCY})
    endif()
    set(FONTS_DEPENDENCY creepster_font)
else()
    message(STATUS "Using cached Creepster font")
endif()

# Nunito - Rounded body text font (variable font)
if(NOT EXISTS "${FONTS_DIR}/Nunito.ttf")
    message(STATUS "Downloading Nunito font...")
    ExternalProject_Add(nunito_font
        URL "https://raw.githubusercontent.com/google/fonts/main/ofl/nunito/Nunito%5Bwght%5D.ttf"
        DOWNLOAD_NO_EXTRACT TRUE
        DOWNLOAD_NAME Nunito.ttf
        PREFIX ${PREBUILT_DIR}/fonts-build/nunito
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
            ${PREBUILT_DIR}/fonts-build/nunito/src/Nunito.ttf
            ${FONTS_DIR}/
    )
    if(FONTS_DEPENDENCY)
        add_dependencies(nunito_font ${FONTS_DEPENDENCY})
    endif()
    set(FONTS_DEPENDENCY nunito_font)
else()
    message(STATUS "Using cached Nunito font")
endif()

# Noto Color Emoji for emoji support
if(NOT EXISTS "${FONTS_DIR}/NotoColorEmoji.ttf")
    message(STATUS "Downloading Noto Color Emoji font...")
    ExternalProject_Add(noto_emoji
        URL https://raw.githubusercontent.com/googlefonts/noto-emoji/main/fonts/NotoColorEmoji.ttf
        DOWNLOAD_NO_EXTRACT TRUE
        DOWNLOAD_NAME NotoColorEmoji.ttf
        PREFIX ${PREBUILT_DIR}/fonts-build/noto
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
            ${PREBUILT_DIR}/fonts-build/noto/src/NotoColorEmoji.ttf
            ${FONTS_DIR}/NotoColorEmoji.ttf
    )
    if(FONTS_DEPENDENCY)
        add_dependencies(noto_emoji ${FONTS_DEPENDENCY})
    endif()
    set(FONTS_DEPENDENCY noto_emoji)
else()
    message(STATUS "Using cached Noto Color Emoji font")
endif()

#######################################
# nlohmann_json (header-only)
#######################################
if(NOT EXISTS "${JSON_STAMP_FILE}")
    message(STATUS "Downloading nlohmann_json...")
    ExternalProject_Add(json_external
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
        GIT_SUBMODULES ""

        PREFIX ${PREBUILT_DIR}/json-build
        SOURCE_DIR ${PREBUILT_DIR}/json-src
        BINARY_DIR ${PREBUILT_DIR}/json-build
        INSTALL_DIR ${JSON_INSTALL_DIR}

        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E make_directory ${JSON_INSTALL_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PREBUILT_DIR}/json-src/include
            ${JSON_INSTALL_DIR}/include
        COMMAND ${CMAKE_COMMAND} -E touch ${JSON_STAMP_FILE}
    )

    set(JSON_DEPENDENCY json_external)
else()
    message(STATUS "Using cached nlohmann_json from ${JSON_INSTALL_DIR}")
    set(JSON_DEPENDENCY "")
endif()

#######################################
# OpenSSL (static library for HTTPS)
#######################################
set(OPENSSL_INSTALL_DIR "${PREBUILT_DIR}/openssl")
set(OPENSSL_STAMP_FILE "${PREBUILT_DIR}/.openssl-stamp")

# Only build OpenSSL for Linux (Windows uses Schannel)
if(TARGET_OS STREQUAL "linux" AND NOT EXISTS "${OPENSSL_STAMP_FILE}")
    message(STATUS "Building OpenSSL (static)...")

    # Determine OpenSSL target platform
    if(TARGET_ARCH STREQUAL "x86_64")
        set(OPENSSL_TARGET "linux-x86_64")
    else()
        set(OPENSSL_TARGET "linux-aarch64")
    endif()

    # Cross-compilation: set CC directly
    set(OPENSSL_CC_ENV "")
    if(IS_CROSS_COMPILE)
        if(TARGET_PLATFORM STREQUAL "linux-x86_64")
            set(OPENSSL_CC_ENV "CC=x86_64-linux-gnu-gcc")
        elseif(TARGET_PLATFORM STREQUAL "linux-arm64")
            set(OPENSSL_CC_ENV "CC=aarch64-linux-gnu-gcc")
        endif()
    endif()

    ExternalProject_Add(openssl_external
        GIT_REPOSITORY https://github.com/openssl/openssl.git
        GIT_TAG openssl-3.2.0
        GIT_SHALLOW TRUE
        GIT_SUBMODULES ""

        PREFIX ${PREBUILT_DIR}/openssl-build
        SOURCE_DIR ${PREBUILT_DIR}/openssl-src
        BINARY_DIR ${PREBUILT_DIR}/openssl-src
        INSTALL_DIR ${OPENSSL_INSTALL_DIR}

        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env ${OPENSSL_CC_ENV}
            <SOURCE_DIR>/Configure ${OPENSSL_TARGET}
            --prefix=${OPENSSL_INSTALL_DIR}
            --openssldir=${OPENSSL_INSTALL_DIR}/ssl
            --libdir=lib
            no-shared
            no-tests
            no-docs
            no-apps
            no-engine
            no-legacy
            no-dso
            no-comp

        BUILD_COMMAND make -j${NPROC} build_libs
        INSTALL_COMMAND make install_sw
        COMMAND ${CMAKE_COMMAND} -E touch ${OPENSSL_STAMP_FILE}

        USES_TERMINAL_BUILD TRUE
    )

    set(OPENSSL_DEPENDENCY openssl_external)
else()
    if(TARGET_OS STREQUAL "linux")
        message(STATUS "Using cached OpenSSL from ${OPENSSL_INSTALL_DIR}")
    endif()
    set(OPENSSL_DEPENDENCY "")
endif()

#######################################
# curl (static library for HTTP requests)
#######################################
if(NOT EXISTS "${CURL_STAMP_FILE}")
    message(STATUS "Building curl (static)...")

    # SSL backend: Use Schannel for Windows, our static OpenSSL for Linux
    if(TARGET_OS STREQUAL "windows")
        set(CURL_SSL_ARGS -DCURL_USE_SCHANNEL=ON -DCURL_USE_OPENSSL=OFF)
    else()
        # Linux: use our statically built OpenSSL
        set(CURL_SSL_ARGS
            -DCURL_USE_OPENSSL=ON
            -DOPENSSL_ROOT_DIR=${OPENSSL_INSTALL_DIR}
            -DOPENSSL_USE_STATIC_LIBS=TRUE
        )
    endif()

    # Cross-compilation: pass compiler directly (toolchain file not always honored)
    set(CURL_CROSS_ARGS "")
    if(IS_CROSS_COMPILE)
        if(TARGET_PLATFORM STREQUAL "linux-x86_64")
            set(CURL_CROSS_ARGS
                -DCMAKE_C_COMPILER=x86_64-linux-gnu-gcc
                -DCMAKE_CXX_COMPILER=x86_64-linux-gnu-g++
                -DCMAKE_SYSTEM_NAME=Linux
            )
        elseif(TARGET_PLATFORM STREQUAL "linux-arm64")
            set(CURL_CROSS_ARGS
                -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc
                -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
                -DCMAKE_SYSTEM_NAME=Linux
            )
        elseif(TARGET_PLATFORM STREQUAL "windows-x86_64")
            set(CURL_CROSS_ARGS
                -DCMAKE_C_COMPILER=/usr/bin/x86_64-w64-mingw32-gcc
                -DCMAKE_CXX_COMPILER=/usr/bin/x86_64-w64-mingw32-g++
                -DCMAKE_SYSTEM_NAME=Windows
            )
        elseif(TARGET_PLATFORM STREQUAL "windows-arm64")
            set(CURL_CROSS_ARGS
                -DCMAKE_C_COMPILER=aarch64-w64-mingw32-clang
                -DCMAKE_CXX_COMPILER=aarch64-w64-mingw32-clang++
                -DCMAKE_SYSTEM_NAME=Windows
            )
        endif()
    endif()

    ExternalProject_Add(curl_external
        GIT_REPOSITORY https://github.com/curl/curl.git
        GIT_TAG curl-8_5_0
        GIT_SHALLOW TRUE

        PREFIX ${PREBUILT_DIR}/curl-build
        SOURCE_DIR ${PREBUILT_DIR}/curl-src
        BINARY_DIR ${PREBUILT_DIR}/curl-build
        INSTALL_DIR ${CURL_INSTALL_DIR}

        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CURL_INSTALL_DIR}
            -DCMAKE_BUILD_TYPE=Release
            -DBUILD_SHARED_LIBS=OFF
            -DBUILD_CURL_EXE=OFF
            ${CURL_SSL_ARGS}
            -DCURL_USE_LIBSSH2=OFF
            -DCURL_ZLIB=OFF
            -DHTTP_ONLY=ON
            -DCURL_DISABLE_OPENSSL_AUTO_LOAD_CONFIG=ON
            -DENABLE_UNIX_SOCKETS=OFF
            ${CURL_CROSS_ARGS}

        BUILD_COMMAND cmake --build <BINARY_DIR> --parallel ${NPROC}
        INSTALL_COMMAND cmake --install <BINARY_DIR>
        COMMAND ${CMAKE_COMMAND} -E touch ${CURL_STAMP_FILE}

        DEPENDS ${OPENSSL_DEPENDENCY}
        USES_TERMINAL_BUILD TRUE
    )

    set(CURL_DEPENDENCY curl_external)
else()
    message(STATUS "Using cached curl from ${CURL_INSTALL_DIR}")
    set(CURL_DEPENDENCY "")
endif()

#######################################
# glog (static library for logging)
#######################################
if(NOT EXISTS "${GLOG_STAMP_FILE}")
    message(STATUS "Building glog (static)...")

    # Cross-compilation: pass compiler directly (toolchain file not always honored)
    set(GLOG_CROSS_ARGS "")
    if(IS_CROSS_COMPILE)
        if(TARGET_PLATFORM STREQUAL "linux-x86_64")
            set(GLOG_CROSS_ARGS
                -DCMAKE_C_COMPILER=x86_64-linux-gnu-gcc
                -DCMAKE_CXX_COMPILER=x86_64-linux-gnu-g++
                -DCMAKE_SYSTEM_NAME=Linux
            )
        elseif(TARGET_PLATFORM STREQUAL "linux-arm64")
            set(GLOG_CROSS_ARGS
                -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc
                -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
                -DCMAKE_SYSTEM_NAME=Linux
            )
        elseif(TARGET_PLATFORM STREQUAL "windows-x86_64")
            set(GLOG_CROSS_ARGS
                -DCMAKE_C_COMPILER=/usr/bin/x86_64-w64-mingw32-gcc
                -DCMAKE_CXX_COMPILER=/usr/bin/x86_64-w64-mingw32-g++
                -DCMAKE_SYSTEM_NAME=Windows
            )
        elseif(TARGET_PLATFORM STREQUAL "windows-arm64")
            set(GLOG_CROSS_ARGS
                -DCMAKE_C_COMPILER=aarch64-w64-mingw32-clang
                -DCMAKE_CXX_COMPILER=aarch64-w64-mingw32-clang++
                -DCMAKE_SYSTEM_NAME=Windows
            )
        endif()
    endif()

    ExternalProject_Add(glog_external
        GIT_REPOSITORY https://github.com/google/glog.git
        GIT_TAG v0.7.1
        GIT_SHALLOW TRUE

        PREFIX ${PREBUILT_DIR}/glog-build
        SOURCE_DIR ${PREBUILT_DIR}/glog-src
        BINARY_DIR ${PREBUILT_DIR}/glog-build
        INSTALL_DIR ${GLOG_INSTALL_DIR}

        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${GLOG_INSTALL_DIR}
            -DCMAKE_BUILD_TYPE=Release
            -DBUILD_SHARED_LIBS=OFF
            -DWITH_GFLAGS=OFF
            -DWITH_GTEST=OFF
            -DWITH_UNWIND=OFF
            ${GLOG_CROSS_ARGS}

        BUILD_COMMAND cmake --build <BINARY_DIR> --parallel ${NPROC}
        INSTALL_COMMAND cmake --install <BINARY_DIR>
        COMMAND ${CMAKE_COMMAND} -E touch ${GLOG_STAMP_FILE}

        USES_TERMINAL_BUILD TRUE
    )

    set(GLOG_DEPENDENCY glog_external)
else()
    message(STATUS "Using cached glog from ${GLOG_INSTALL_DIR}")
    set(GLOG_DEPENDENCY "")
endif()

#######################################
# Dependency umbrella target
#######################################
add_custom_target(deps)
if(QT_DEPENDENCY)
    add_dependencies(deps ${QT_DEPENDENCY})
endif()
if(FONTS_DEPENDENCY)
    add_dependencies(deps ${FONTS_DEPENDENCY})
endif()
if(JSON_DEPENDENCY)
    add_dependencies(deps ${JSON_DEPENDENCY})
endif()
if(OPENSSL_DEPENDENCY)
    add_dependencies(deps ${OPENSSL_DEPENDENCY})
endif()
if(CURL_DEPENDENCY)
    add_dependencies(deps ${CURL_DEPENDENCY})
endif()
if(GLOG_DEPENDENCY)
    add_dependencies(deps ${GLOG_DEPENDENCY})
endif()

#######################################
# Main Project
#######################################
set(APP_CMAKE_ARGS
    -DCMAKE_PREFIX_PATH=${QT_INSTALL_DIR}
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
    -DPREBUILT_DIR=${PREBUILT_DIR}
    -DTARGET_PLATFORM=${TARGET_PLATFORM}
    -DJSON_ROOT=${JSON_INSTALL_DIR}
    -DCURL_ROOT=${CURL_INSTALL_DIR}
    -DOPENSSL_ROOT=${OPENSSL_INSTALL_DIR}
    -Dglog_ROOT=${GLOG_INSTALL_DIR}
    -DUSE_PREBUILT_DEPS=ON
)

# Add toolchain file for cross-compilation
if(IS_CROSS_COMPILE)
    set(TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/toolchains/${TARGET_PLATFORM}.cmake")
    if(EXISTS "${TOOLCHAIN_FILE}")
        list(APPEND APP_CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE})
    endif()
endif()

ExternalProject_Add(rick_and_morty_viewer
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/src-project
    BINARY_DIR ${CMAKE_BINARY_DIR}/app-build
    INSTALL_DIR ${CMAKE_BINARY_DIR}/install

    CMAKE_ARGS ${APP_CMAKE_ARGS}

    BUILD_COMMAND cmake --build <BINARY_DIR> --parallel ${NPROC}
    INSTALL_COMMAND ""

    DEPENDS ${QT_DEPENDENCY} ${FONTS_DEPENDENCY} ${JSON_DEPENDENCY} ${CURL_DEPENDENCY} ${GLOG_DEPENDENCY}
)

#######################################
# Distribution target
#######################################
add_custom_target(distribute
    DEPENDS rick_and_morty_viewer
    COMMAND ${CMAKE_COMMAND} -E echo "Creating distribution bundle for ${TARGET_PLATFORM}..."
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/plugins
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/qml

    # Copy executable
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/app-build/RickAndMortyViewer${EXE_EXTENSION}
        ${DIST_DIR}/

    # Copy Qt libraries
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${QT_INSTALL_DIR}/lib
        ${DIST_DIR}/lib

    # Copy Qt plugins
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${QT_INSTALL_DIR}/plugins
        ${DIST_DIR}/plugins

    # Copy Qt QML modules (QtQuick runtime - required for QtQuick.Controls etc.)
    # Note: App's own QML files are compiled into the binary via resources.qrc
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${QT_INSTALL_DIR}/qml
        ${DIST_DIR}/qml

    # Copy fonts and fontconfig configuration
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/share/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${FONTS_DIR}
        ${DIST_DIR}/share/fonts
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/etc/fonts
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/cmake/fonts.conf
        ${DIST_DIR}/etc/fonts/fonts.conf

    COMMENT "Creating distribution in ${DIST_DIR}"
)

# Copy system libraries that Qt plugins depend on (Linux native builds only)
# Cross-compiled builds use Qt's bundled libraries (-qt-freetype, -qt-pcre, etc.)
if(TARGET_OS STREQUAL "linux" AND NOT IS_CROSS_COMPILE)
    # Find system library directory based on architecture
    if(TARGET_ARCH STREQUAL "x86_64")
        set(SYSTEM_LIB_DIR "/usr/lib/x86_64-linux-gnu")
    else()
        set(SYSTEM_LIB_DIR "/usr/lib/aarch64-linux-gnu")
    endif()

    add_custom_command(TARGET distribute POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying system libraries..."
        # libjpeg for Qt JPEG plugin
        COMMAND sh -c "cp -a ${SYSTEM_LIB_DIR}/libjpeg.so* ${DIST_DIR}/lib/"
        # libpcre2-16 for Qt Core
        COMMAND sh -c "cp -a ${SYSTEM_LIB_DIR}/libpcre2-16.so* ${DIST_DIR}/lib/"
        # Note: OpenSSL is now statically linked, no need to copy .so files
        COMMENT "Copying system libraries for Qt plugins"
    )
endif()

# Copy appropriate launcher script
if(TARGET_OS STREQUAL "linux")
    add_custom_command(TARGET distribute POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/cmake/run.sh.in
            ${DIST_DIR}/run.sh
        COMMAND chmod +x ${DIST_DIR}/run.sh
    )
elseif(TARGET_OS STREQUAL "windows")
    add_custom_command(TARGET distribute POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/cmake/run.bat.in
            ${DIST_DIR}/run.bat
    )
endif()

#######################################
# AppImage target (Linux only)
#######################################
if(TARGET_OS STREQUAL "linux")
    # Determine AppImage architecture name
    if(TARGET_ARCH STREQUAL "x86_64")
        set(APPIMAGE_ARCH "x86_64")
    else()
        set(APPIMAGE_ARCH "aarch64")
    endif()

    set(APPIMAGE_DIR "${CMAKE_SOURCE_DIR}/.distribute")
    set(APPIMAGE_NAME "RickAndMortyViewer-${APPIMAGE_ARCH}.AppImage")

    add_custom_target(appimage
        DEPENDS distribute
        COMMAND ${CMAKE_COMMAND} -E echo "Creating AppImage..."

        # Create AppDir structure
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/AppDir/usr/bin
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/AppDir/usr/lib
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/AppDir/usr/share/applications
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/AppDir/usr/share/icons/hicolor/256x256/apps

        # Copy binary
        COMMAND ${CMAKE_COMMAND} -E copy
            ${DIST_DIR}/RickAndMortyViewer
            ${DIST_DIR}/AppDir/usr/bin/

        # Copy libraries
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${DIST_DIR}/lib
            ${DIST_DIR}/AppDir/usr/lib

        # Copy plugins
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${DIST_DIR}/plugins
            ${DIST_DIR}/AppDir/usr/plugins

        # Copy Qt QML runtime modules (Qt's implementation of QtQuick.Controls etc.)
        # Note: These are Qt's runtime dependencies, not the app's QML files
        # The app's QML (Main.qml, Theme.qml, etc.) is compiled into the binary
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${DIST_DIR}/qml
            ${DIST_DIR}/AppDir/usr/qml

        # Copy fonts and fontconfig config
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/AppDir/usr/share/fonts
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${DIST_DIR}/share/fonts
            ${DIST_DIR}/AppDir/usr/share/fonts
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/AppDir/usr/etc/fonts
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${DIST_DIR}/etc/fonts
            ${DIST_DIR}/AppDir/usr/etc/fonts

        # Copy AppImage assets
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/appimage/RickAndMortyViewer.desktop
            ${DIST_DIR}/AppDir/usr/share/applications/
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/appimage/RickAndMortyViewer.desktop
            ${DIST_DIR}/AppDir/
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/appimage/icon.png
            ${DIST_DIR}/AppDir/usr/share/icons/hicolor/256x256/apps/RickAndMortyViewer.png
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/appimage/icon.png
            ${DIST_DIR}/AppDir/RickAndMortyViewer.png
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/appimage/AppRun
            ${DIST_DIR}/AppDir/
        COMMAND chmod +x ${DIST_DIR}/AppDir/AppRun
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/appimage/COPYING
            ${DIST_DIR}/AppDir/

        # Bundle glibc for portability (works for native builds, cross builds need docker)
        COMMAND bash ${CMAKE_SOURCE_DIR}/cmake/bundle-glibc.sh ${DIST_DIR}/AppDir ${APPIMAGE_ARCH}

        # Create AppImage using appimagetool
        # Note: appimagetool outputs to second argument path, linuxdeploy outputs to current dir
        COMMAND ${CMAKE_COMMAND} -E chdir ${APPIMAGE_DIR}
            ${CMAKE_COMMAND} -E env ARCH=${APPIMAGE_ARCH}
            appimagetool ${DIST_DIR}/AppDir ${APPIMAGE_NAME}

        COMMENT "Creating AppImage: ${APPIMAGE_DIR}/${APPIMAGE_NAME}"
    )
endif()

#######################################
# Tests (optional)
#######################################
option(BUILD_TESTS "Build the test suite" OFF)

if(BUILD_TESTS)
    message(STATUS "Building test suite...")

    # Tests are built as a separate ExternalProject to use consistent dependency versions
    set(TESTS_CMAKE_ARGS
        -DCMAKE_PREFIX_PATH=${QT_INSTALL_DIR}
        -DCMAKE_BUILD_TYPE=Release
        -DPREBUILT_DIR=${PREBUILT_DIR}
        -DTARGET_PLATFORM=${TARGET_PLATFORM}
        -DCURL_ROOT=${CURL_INSTALL_DIR}
        -DOPENSSL_ROOT=${OPENSSL_INSTALL_DIR}
        -Dglog_ROOT=${GLOG_INSTALL_DIR}
        -DUSE_PREBUILT_DEPS=ON
    )

    # Add toolchain file for cross-compilation
    if(IS_CROSS_COMPILE)
        if(EXISTS "${TOOLCHAIN_FILE}")
            list(APPEND TESTS_CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE})
        endif()
    endif()

    ExternalProject_Add(tests
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/tests
        BINARY_DIR ${CMAKE_BINARY_DIR}/tests-build

        CMAKE_ARGS ${TESTS_CMAKE_ARGS}

        BUILD_COMMAND cmake --build <BINARY_DIR> --parallel ${NPROC}
        INSTALL_COMMAND ""

        DEPENDS ${QT_DEPENDENCY} ${CURL_DEPENDENCY} ${GLOG_DEPENDENCY}
        USES_TERMINAL_BUILD TRUE
    )

    # Convenience target to run all tests
    add_custom_target(check
        DEPENDS tests
        COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR}/tests-build --output-on-failure --parallel
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests-build
        COMMENT "Running all tests..."
    )
endif()

#######################################
# Build Instructions
#######################################
message(STATUS "")
message(STATUS "=== Build Instructions ===")
message(STATUS "  Using build.sh (recommended):")
message(STATUS "    ./build.sh deps           # Build Qt + deps (~2-3 hours)")
message(STATUS "    ./build.sh app            # Build application (~5 min)")
message(STATUS "    ./build.sh distribute     # Create distribution")
message(STATUS "    ./build.sh appimage       # Create AppImage")
message(STATUS "    ./build.sh test           # Build and run tests")
message(STATUS "")
message(STATUS "  Manual cmake:")
message(STATUS "    cmake -B .build/${TARGET_PLATFORM} -DTARGET_PLATFORM=${TARGET_PLATFORM}")
message(STATUS "    cmake --build .build/${TARGET_PLATFORM}")
message(STATUS "    cmake --build .build/${TARGET_PLATFORM} --target distribute")
message(STATUS "")
message(STATUS "  Testing (manual cmake):")
message(STATUS "    cmake -B .build/${TARGET_PLATFORM} -DTARGET_PLATFORM=${TARGET_PLATFORM} -DBUILD_TESTS=ON")
message(STATUS "    cmake --build .build/${TARGET_PLATFORM} --target tests")
message(STATUS "    cmake --build .build/${TARGET_PLATFORM} --target check")
if(TARGET_OS STREQUAL "linux")
    message(STATUS "    ./.distribute/${TARGET_PLATFORM}/run.sh")
else()
    message(STATUS "    ./.distribute/${TARGET_PLATFORM}/run.bat")
endif()
message(STATUS "")
message(STATUS "Available platforms: ${VALID_PLATFORMS}")
message(STATUS "")
