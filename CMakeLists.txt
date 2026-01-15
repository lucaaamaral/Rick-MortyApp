cmake_minimum_required(VERSION 3.16)

project(RickAndMortyViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick QuickControls2)
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(glog REQUIRED)

# Core library (backend - no Qt dependency except for threading)
add_library(core STATIC
    src/core/Models.h
    src/core/Observer.h
    src/core/ApiClient.h
    src/core/ApiClient.cpp
    src/core/DataStore.h
    src/core/DataStore.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(core PUBLIC
    CURL::libcurl
    nlohmann_json::nlohmann_json
    glog::glog
)

# UI library (Qt/QML specific)
add_library(ui STATIC
    src/ui/QmlBridge.h
    src/ui/QmlBridge.cpp
    src/ui/EpisodeModel.h
    src/ui/EpisodeModel.cpp
    src/ui/CharacterModel.h
    src/ui/CharacterModel.cpp
)

target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ui PUBLIC
    core
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
)

# Resources
qt_add_resources(RESOURCES resources/resources.qrc)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ui
    Qt6::Widgets
)

# Copy QML files for development
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/qml
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
