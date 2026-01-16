#######################################
# System Tests (End-to-End)
#######################################

# Collect all system test source files
set(SYSTEM_TEST_SOURCES
    test_placeholder.cpp
)

# Create the system test executable
add_executable(tests_system ${SYSTEM_TEST_SOURCES})

# Link against GoogleTest, GMock, and project libraries
# Note: ui library only included when BUILD_UI_TESTS is ON
set(SYSTEM_LIBS
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    core
)
if(TARGET ui)
    list(APPEND SYSTEM_LIBS ui)
endif()
target_link_libraries(tests_system PRIVATE ${SYSTEM_LIBS})

# Include directories for test sources
target_include_directories(tests_system PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(tests_system
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    PROPERTIES
        LABELS "system"
)
