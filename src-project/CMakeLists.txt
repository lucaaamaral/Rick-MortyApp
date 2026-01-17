cmake_minimum_required(VERSION 3.16)

project(RickAndMortyViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source directory (parent)
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../resources")

# Find Qt6 (from prebuilt or CMAKE_PREFIX_PATH)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick QuickControls2)
message(STATUS "Found Qt6 ${Qt6_VERSION} at ${Qt6_DIR}")

# Threads (needed by glog when statically linked)
find_package(Threads REQUIRED)

# Option to use prebuilt dependencies from superbuild
option(USE_PREBUILT_DEPS "Use prebuilt curl/glog from superbuild" OFF)

include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# nlohmann_json (always fetch - header only, fast)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")

if(USE_PREBUILT_DEPS)
    message(STATUS "Using prebuilt dependencies from superbuild")

    # For cross-compilation, curl's CMake config may fail to find OpenSSL
    # Use manual library specification instead
    if(DEFINED CURL_ROOT)
        set(CURL_INCLUDE_DIRS "${CURL_ROOT}/include")
        if(WIN32)
            set(CURL_LIBRARIES "${CURL_ROOT}/lib/libcurl.a")
        else()
            set(CURL_LIBRARIES "${CURL_ROOT}/lib/libcurl.a")
        endif()
        message(STATUS "Using prebuilt CURL from ${CURL_ROOT}")

        # Create imported target for compatibility
        add_library(CURL::libcurl STATIC IMPORTED)
        set_target_properties(CURL::libcurl PROPERTIES
            IMPORTED_LOCATION "${CURL_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}"
        )
    else()
        find_package(CURL REQUIRED)
        message(STATUS "Found CURL ${CURL_VERSION_STRING} at ${CURL_INCLUDE_DIRS}")
    endif()

    # Find prebuilt glog
    find_package(glog REQUIRED)
    message(STATUS "Found glog at ${glog_DIR}")

    # Use prebuilt nlohmann_json when available
    if(DEFINED JSON_ROOT)
        message(STATUS "Using prebuilt nlohmann_json from ${JSON_ROOT}")
        add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
        set_target_properties(nlohmann_json::nlohmann_json PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${JSON_ROOT}/include"
        )
    else()
        message(STATUS "Fetching nlohmann_json...")
        FetchContent_MakeAvailable(json)
    endif()
else()
    message(STATUS "Building dependencies from source via FetchContent")

    # glog
    FetchContent_Declare(glog
        GIT_REPOSITORY https://github.com/google/glog.git
        GIT_TAG v0.7.1
        GIT_SHALLOW TRUE
    )
    set(WITH_GFLAGS OFF CACHE INTERNAL "")
    set(WITH_GTEST OFF CACHE INTERNAL "")
    set(WITH_UNWIND OFF CACHE INTERNAL "")
    set(BUILD_TESTING OFF CACHE INTERNAL "")

    # CURL (static build)
    FetchContent_Declare(curl
        GIT_REPOSITORY https://github.com/curl/curl.git
        GIT_TAG curl-8_5_0
        GIT_SHALLOW TRUE
    )
    set(CURL_STATICLIB ON CACHE INTERNAL "")
    set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
    set(BUILD_TESTING OFF CACHE INTERNAL "")
    set(CURL_DISABLE_TESTS ON CACHE INTERNAL "")
    set(CURL_USE_OPENSSL ON CACHE INTERNAL "")
    set(CURL_USE_LIBSSH2 OFF CACHE INTERNAL "")
    set(CURL_ZLIB OFF CACHE INTERNAL "")

    message(STATUS "Fetching dependencies...")
    FetchContent_MakeAvailable(json glog curl)
endif()

# Static linking for GCC/Clang (except Qt)
if(NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

# Core library (backend)
add_library(core STATIC
    ${SRC_DIR}/core/Models.h
    ${SRC_DIR}/core/Observer.h
    ${SRC_DIR}/core/IHttpClient.h
    ${SRC_DIR}/core/CurlHttpClient.h
    ${SRC_DIR}/core/CurlHttpClient.cpp
    ${SRC_DIR}/core/ApiClient.h
    ${SRC_DIR}/core/ApiClient.cpp
    ${SRC_DIR}/core/DataStore.h
    ${SRC_DIR}/core/DataStore.cpp
)

target_include_directories(core PUBLIC ${SRC_DIR})

if(USE_PREBUILT_DEPS)
    target_link_libraries(core PUBLIC
        CURL::libcurl
        nlohmann_json::nlohmann_json
        glog::glog
    )
    # Link static OpenSSL for Linux builds (Windows uses Schannel via curl)
    if(NOT WIN32 AND DEFINED OPENSSL_ROOT)
        target_link_libraries(core PUBLIC
            ${OPENSSL_ROOT}/lib/libssl.a
            ${OPENSSL_ROOT}/lib/libcrypto.a
        )
    endif()
else()
    target_link_libraries(core PUBLIC
        CURL::libcurl
        nlohmann_json::nlohmann_json
        glog::glog
    )
endif()
target_compile_definitions(core PUBLIC CURL_STATICLIB)
target_link_libraries(core PUBLIC Threads::Threads)
if(WIN32)
    target_link_libraries(core PUBLIC bcrypt crypt32)
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(core PUBLIC pthread)
endif()

# UI library (Qt/QML specific)
add_library(ui STATIC
    ${SRC_DIR}/ui/QmlBridge.h
    ${SRC_DIR}/ui/QmlBridge.cpp
    ${SRC_DIR}/ui/EpisodeModel.h
    ${SRC_DIR}/ui/EpisodeModel.cpp
    ${SRC_DIR}/ui/CharacterModel.h
    ${SRC_DIR}/ui/CharacterModel.cpp
)

target_include_directories(ui PUBLIC ${SRC_DIR})
target_link_libraries(ui PUBLIC
    core
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
)

# Resources
qt_add_resources(RESOURCES ${RESOURCES_DIR}/resources.qrc)

# Main executable
add_executable(${PROJECT_NAME}
    ${SRC_DIR}/main.cpp
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ui
    Qt6::Widgets
    Threads::Threads
)

# Copy QML files for development
file(COPY ${RESOURCES_DIR}/qml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
