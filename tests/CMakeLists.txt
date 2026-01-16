cmake_minimum_required(VERSION 3.16)

project(RickAndMortyViewerTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#######################################
# Enable testing
#######################################
enable_testing()

#######################################
# Fetch GoogleTest (v1.14.0)
#######################################
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
)

# Prevent GoogleTest from overriding our compiler/linker options
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

#######################################
# Fetch RapidCheck (latest from GitHub)
#######################################
FetchContent_Declare(
    rapidcheck
    GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

set(RC_ENABLE_GTEST ON CACHE BOOL "" FORCE)
set(RC_ENABLE_GMOCK ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(rapidcheck)

#######################################
# Find project dependencies
#######################################
# Find Qt6 (optional, required for UI tests)
find_package(Qt6 COMPONENTS Core Gui Widgets Quick QuickControls2 QUIET)

# Find nlohmann_json
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(json)

# Find curl and glog (from prebuilt or system)
if(USE_PREBUILT_DEPS AND DEFINED CURL_ROOT)
    set(CURL_INCLUDE_DIRS "${CURL_ROOT}/include")
    set(CURL_LIBRARIES "${CURL_ROOT}/lib/libcurl.a")
    add_library(CURL::libcurl STATIC IMPORTED)
    set_target_properties(CURL::libcurl PROPERTIES
        IMPORTED_LOCATION "${CURL_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}"
    )
else()
    find_package(CURL REQUIRED)
endif()

find_package(glog REQUIRED)

#######################################
# Core library (from src-project)
#######################################
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")

add_library(core STATIC
    ${SRC_DIR}/core/Models.h
    ${SRC_DIR}/core/Observer.h
    ${SRC_DIR}/core/ApiClient.h
    ${SRC_DIR}/core/ApiClient.cpp
    ${SRC_DIR}/core/DataStore.h
    ${SRC_DIR}/core/DataStore.cpp
)

target_include_directories(core PUBLIC ${SRC_DIR})
target_link_libraries(core PUBLIC
    CURL::libcurl
    nlohmann_json::nlohmann_json
    glog::glog
)
target_compile_definitions(core PUBLIC CURL_STATICLIB)

# Link OpenSSL for Linux builds
if(NOT WIN32 AND DEFINED OPENSSL_ROOT)
    target_link_libraries(core PUBLIC
        ${OPENSSL_ROOT}/lib/libssl.a
        ${OPENSSL_ROOT}/lib/libcrypto.a
    )
endif()

#######################################
# UI library (from src-project) - Optional
#######################################
option(BUILD_UI_TESTS "Build tests that require Qt UI library" OFF)

if(BUILD_UI_TESTS)
    set(CMAKE_AUTOMOC ON)

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
endif()

#######################################
# Test fakes library (must be before test subdirectories)
#######################################
add_subdirectory(fakes)

#######################################
# Add test subdirectories
#######################################
add_subdirectory(unit)
add_subdirectory(property)
add_subdirectory(integration)
add_subdirectory(system)

#######################################
# Convenience target to run all tests
#######################################
set(ALL_TEST_TARGETS tests_unit tests_property tests_integration)
if(BUILD_UI_TESTS)
    list(APPEND ALL_TEST_TARGETS tests_system)
endif()

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --parallel
    DEPENDS ${ALL_TEST_TARGETS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests..."
)
