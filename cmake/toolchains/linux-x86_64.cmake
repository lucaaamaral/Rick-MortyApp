# Cross-compilation toolchain for Linux x86_64
# Requires: sudo apt install gcc-x86-64-linux-gnu g++-x86-64-linux-gnu

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER x86_64-linux-gnu-g++)

set(CMAKE_SYSROOT /)
set(CMAKE_FIND_ROOT_PATH
    /usr/x86_64-linux-gnu
    /usr/lib/x86_64-linux-gnu
    /lib/x86_64-linux-gnu
)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# Use BOTH to allow finding packages via CMAKE_PREFIX_PATH (for prebuilt deps)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
