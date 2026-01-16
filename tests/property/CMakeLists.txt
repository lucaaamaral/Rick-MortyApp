#######################################
# Property-Based Tests (RapidCheck)
#######################################

# Collect all property test source files
set(PROPERTY_TEST_SOURCES
    test_placeholder.cpp
)

# Create the property test executable
add_executable(tests_property ${PROPERTY_TEST_SOURCES})

# Link against GoogleTest, GMock, RapidCheck, and project libraries
# Note: ui library only included when BUILD_UI_TESTS is ON
set(PROPERTY_LIBS
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    rapidcheck
    rapidcheck_gtest
    core
    test_fakes
)
if(TARGET ui)
    list(APPEND PROPERTY_LIBS ui)
endif()
target_link_libraries(tests_property PRIVATE ${PROPERTY_LIBS})

# Include directories for test sources
target_include_directories(tests_property PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Register tests with CTest
include(GoogleTest)
# Use DISCOVERY_MODE PRE_TEST to avoid running cross-compiled binaries at configure time
gtest_discover_tests(tests_property
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    PROPERTIES
        LABELS "property"
    DISCOVERY_MODE PRE_TEST
)
