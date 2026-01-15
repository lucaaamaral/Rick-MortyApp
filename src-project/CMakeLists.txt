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

    # Find prebuilt curl
    find_package(CURL REQUIRED)
    message(STATUS "Found CURL ${CURL_VERSION_STRING} at ${CURL_INCLUDE_DIRS}")

    # Find prebuilt glog
    find_package(glog REQUIRED)
    message(STATUS "Found glog at ${glog_DIR}")

    # Only fetch json
    message(STATUS "Fetching nlohmann_json...")
    FetchContent_MakeAvailable(json)
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
        ssl
        crypto
    )
else()
    target_link_libraries(core PUBLIC
        CURL::libcurl
        nlohmann_json::nlohmann_json
        glog::glog
    )
endif()
target_compile_definitions(core PUBLIC CURL_STATICLIB)

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
)

# Copy QML files for development
file(COPY ${RESOURCES_DIR}/qml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
