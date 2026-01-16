#######################################
# Test Fakes Library
#######################################

# Collect all fake source files
set(FAKES_SOURCES
    FakeHttpClient.cpp
)

set(FAKES_HEADERS
    FakeHttpClient.h
)

# Create the fakes static library
add_library(test_fakes STATIC ${FAKES_SOURCES} ${FAKES_HEADERS})

# Link against core library (for IHttpClient interface)
target_link_libraries(test_fakes PUBLIC
    core
)

# Include directories
target_include_directories(test_fakes PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Ensure C++17 standard
target_compile_features(test_fakes PUBLIC cxx_std_17)
