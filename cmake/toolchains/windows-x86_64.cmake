# Cross-compilation toolchain for Windows x86_64 using MinGW-w64 (POSIX threading)
# Requires: sudo apt install mingw-w64
# Uses the POSIX threading variant for C++11 std::thread support

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Use POSIX threading variant for full C++11 threading support
# The -posix variant uses winpthreads which enables std::thread, std::mutex, etc.
set(CMAKE_C_COMPILER /usr/bin/x86_64-w64-mingw32-gcc-posix)
set(CMAKE_CXX_COMPILER /usr/bin/x86_64-w64-mingw32-g++-posix)
set(CMAKE_RC_COMPILER /usr/bin/x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# Use BOTH to allow finding packages via CMAKE_PREFIX_PATH (for prebuilt deps)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

# Windows-specific settings
set(CMAKE_EXECUTABLE_SUFFIX ".exe")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
