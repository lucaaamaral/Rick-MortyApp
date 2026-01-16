#######################################
# Unit Tests
#######################################

# Collect all test source files
set(UNIT_TEST_SOURCES
    test_placeholder.cpp
    core/url_extraction_test.cpp
    core/episode_parsing_test.cpp
    core/character_parsing_test.cpp
)

# Create the unit test executable
add_executable(tests_unit ${UNIT_TEST_SOURCES})

# Link against GoogleTest, GMock, and project libraries
# Note: ui library excluded for now (requires Qt runtime)
target_link_libraries(tests_unit PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    core
    test_fakes
)

# Include directories for test sources
target_include_directories(tests_unit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(tests_unit
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    PROPERTIES
        LABELS "unit"
)
