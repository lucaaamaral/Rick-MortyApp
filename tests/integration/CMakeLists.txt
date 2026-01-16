#######################################
# Integration Tests
#######################################

# Collect all integration test source files
set(INTEGRATION_TEST_SOURCES
    test_placeholder.cpp
)

# Create the integration test executable
add_executable(tests_integration ${INTEGRATION_TEST_SOURCES})

# Link against GoogleTest, GMock, and project libraries
# Note: ui library only included when BUILD_UI_TESTS is ON
set(INTEGRATION_LIBS
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    core
    test_fakes
)
if(TARGET ui)
    list(APPEND INTEGRATION_LIBS ui)
endif()
target_link_libraries(tests_integration PRIVATE ${INTEGRATION_LIBS})

# Include directories for test sources
target_include_directories(tests_integration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Register tests with CTest
include(GoogleTest)
# Use DISCOVERY_MODE PRE_TEST to avoid running cross-compiled binaries at configure time
gtest_discover_tests(tests_integration
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    PROPERTIES
        LABELS "integration"
    DISCOVERY_MODE PRE_TEST
)
