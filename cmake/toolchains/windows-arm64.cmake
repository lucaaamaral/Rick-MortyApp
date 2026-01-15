# Cross-compilation toolchain for Windows ARM64 using llvm-mingw
# Requires: llvm-mingw from https://github.com/mstorsjo/llvm-mingw/releases

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Assumes llvm-mingw is in PATH or set LLVM_MINGW_PATH
if(DEFINED ENV{LLVM_MINGW_PATH})
    set(TOOLCHAIN_PREFIX "$ENV{LLVM_MINGW_PATH}/bin/aarch64-w64-mingw32-")
else()
    set(TOOLCHAIN_PREFIX "aarch64-w64-mingw32-")
endif()

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}windres)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Windows-specific settings
set(CMAKE_EXECUTABLE_SUFFIX ".exe")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
