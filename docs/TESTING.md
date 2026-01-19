# Testing Guide

This document describes the test infrastructure, test categories, and how to run and write tests for the Rick and Morty Viewer application.

## Overview

The project uses a comprehensive testing strategy with multiple test types:

| Test Type | Framework | Purpose |
|-----------|-----------|---------|
| Unit Tests | Google Test | Test individual functions and classes in isolation |
| Integration Tests | Google Test + Mocks | Test component interactions with mocked dependencies |
| Property Tests | RapidCheck | Generative testing with random inputs |
| System Tests | Google Test | End-to-end testing (placeholder) |

## Test Dependencies

All test dependencies are fetched automatically via CMake `FetchContent`:

| Dependency | Version | Purpose |
|------------|---------|---------|
| Google Test | 1.14.0 | Test framework and assertions |
| Google Mock | 1.14.0 | Mocking framework |
| RapidCheck | latest | Property-based testing |
| nlohmann/json | 3.11.3 | JSON parsing for test fixtures |

## Running Tests

### Via build.sh

```bash
# Build and run all tests
./build.sh test
```

### Via CMake

```bash
# Configure with tests enabled
cmake -B .build/tests -DBUILD_TESTS=ON

# Build test targets
cmake --build .build/tests --target tests

# Run all tests
cd .build/tests/tests-build && ctest --output-on-failure

# Run specific test categories
ctest -L unit              # Unit tests only
ctest -L integration       # Integration tests only
ctest -L property          # Property tests only
ctest -L system            # System tests only

# Run specific test suite
ctest -R UrlExtraction     # Tests matching "UrlExtraction"
ctest -R Character         # Tests matching "Character"

# Verbose output
ctest -V

# Run tests in parallel
ctest -j4
```

### Direct Test Execution

```bash
# Run unit test binary directly
.build/tests/tests-build/unit/tests_unit

# Run with Google Test filters
.build/tests/tests-build/unit/tests_unit --gtest_filter="EpisodeParsingTest.*"
.build/tests/tests-build/unit/tests_unit --gtest_filter="*Invalid*"

# List all tests
.build/tests/tests-build/unit/tests_unit --gtest_list_tests
```

## Test Directory Structure

```
tests/
├── CMakeLists.txt           # Main test CMake configuration
├── unit/                    # Unit tests
│   ├── CMakeLists.txt
│   ├── test_placeholder.cpp # Placeholder test
│   └── core/
│       ├── url_extraction_test.cpp    # URL parsing tests
│       ├── episode_parsing_test.cpp   # Episode JSON parsing tests
│       └── character_parsing_test.cpp # Character JSON parsing tests
├── integration/             # Integration tests
│   ├── CMakeLists.txt
│   └── test_placeholder.cpp
├── property/                # Property-based tests
│   ├── CMakeLists.txt
│   └── test_placeholder.cpp
├── system/                  # System/E2E tests
│   ├── CMakeLists.txt
│   └── test_placeholder.cpp
├── fakes/                   # Test doubles (fakes)
│   ├── CMakeLists.txt
│   ├── FakeHttpClient.h
│   └── FakeHttpClient.cpp
├── mocks/                   # GMock mocks
│   └── MockDataObserver.h
└── fixtures/                # Test data
    └── json/
        ├── characters/
        │   ├── single_character.json
        │   ├── character_batch.json
        │   └── malformed_character.json
        ├── episodes/
        │   ├── single_episode.json
        │   ├── episode_page_1.json
        │   └── malformed_episode.json
        └── locations/
            └── single_location.json
```

## Test Infrastructure

### FakeHttpClient

The `FakeHttpClient` provides a configurable test double for HTTP requests:

```cpp
#include "fakes/FakeHttpClient.h"

// Create fake with predefined responses
FakeHttpClient fake;
fake.whenUrl("https://rickandmortyapi.com/api/character/1")
    .thenReturn(R"({"id": 1, "name": "Rick"})");

// Load response from fixture file
fake.whenUrl("https://rickandmortyapi.com/api/episode/1")
    .thenReturnFixture("episodes/single_episode.json");

// Simulate errors
fake.whenUrl("https://rickandmortyapi.com/api/character/999")
    .thenThrow(HttpException(HttpException::Type::NotFound, "Not found"));

// Simulate network timeout
fake.whenUrlMatching(".*api/location.*")
    .thenThrow(HttpException(HttpException::Type::Timeout, "Request timed out"));

// Inject into ApiClient
ApiClient client(std::make_unique<FakeHttpClient>(std::move(fake)));
```

### MockDataObserver

The `MockDataObserver` uses Google Mock for verifying observer callbacks:

```cpp
#include "mocks/MockDataObserver.h"

MockDataObserver observer;

// Expect specific callback
EXPECT_CALL(observer, onEpisodesLoaded(testing::_))
    .Times(1);

// Use custom matchers
EXPECT_CALL(observer, onCharacterLoaded(CharacterWithName("Rick Sanchez")))
    .Times(1);

// Verify no errors
EXPECT_CALL(observer, onError(testing::_))
    .Times(0);

dataStore.addObserver(&observer);
dataStore.loadEpisodes();
```

### Custom GMock Matchers

The `MockDataObserver.h` provides domain-specific matchers:

```cpp
// Character matchers
CharacterWithId(1)
CharacterWithName("Rick Sanchez")
CharacterWithStatus(CharacterStatus::Alive)
CharacterWithSpecies("Human")
CharacterWithGender(Gender::Male)

// Episode matchers
EpisodeWithId(1)
EpisodeWithName("Pilot")
EpisodeWithCode("S01E01")
EpisodeWithSeason(1)
EpisodeInSeason(1)

// Collection matchers
HasCharacterCount(5)
HasEpisodeCount(10)
```

## Writing Tests

### Unit Test Example

```cpp
#include <gtest/gtest.h>
#include "core/Models.h"

namespace rickmorty {

class UrlExtractionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(UrlExtractionTest, ExtractsIdFromValidUrl) {
    std::string url = "https://rickandmortyapi.com/api/character/42";
    EXPECT_EQ(extractIdFromUrl(url), 42);
}

TEST_F(UrlExtractionTest, ReturnsMinusOneForInvalidUrl) {
    std::string url = "invalid";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

} // namespace rickmorty
```

### Integration Test Example

```cpp
#include <gtest/gtest.h>
#include "core/ApiClient.h"
#include "fakes/FakeHttpClient.h"

namespace rickmorty {

class ApiClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        fake_ = std::make_unique<FakeHttpClient>();
    }

    std::unique_ptr<FakeHttpClient> fake_;
};

TEST_F(ApiClientTest, FetchesCharacterById) {
    fake_->whenUrl("https://rickandmortyapi.com/api/character/1")
        .thenReturnFixture("characters/single_character.json");

    ApiClient client(std::move(fake_));
    auto character = client.getCharacter(1);

    EXPECT_EQ(character.id, 1);
    EXPECT_EQ(character.name, "Rick Sanchez");
}

TEST_F(ApiClientTest, ThrowsOnNotFound) {
    fake_->whenUrl("https://rickandmortyapi.com/api/character/999")
        .thenThrow(HttpException(HttpException::Type::NotFound, "Not found"));

    ApiClient client(std::move(fake_));
    EXPECT_THROW(client.getCharacter(999), HttpException);
}

} // namespace rickmorty
```

### Property Test Example

```cpp
#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>
#include "core/Models.h"

namespace rickmorty {

RC_GTEST_PROP(UrlExtraction, ExtractsPositiveIds, (int id)) {
    RC_PRE(id > 0 && id < 10000);

    std::string url = "https://rickandmortyapi.com/api/character/" + std::to_string(id);
    RC_ASSERT(extractIdFromUrl(url) == id);
}

RC_GTEST_PROP(UrlExtraction, HandlesAnyTrailingSlashCount, (int slashCount)) {
    RC_PRE(slashCount >= 0 && slashCount < 10);

    std::string url = "https://example.com" + std::string(slashCount, '/');
    // Should not crash
    int result = extractIdFromUrl(url);
    RC_ASSERT(result >= -1);
}

} // namespace rickmorty
```

## Test Categories

### URL Extraction Tests (25 tests)

Tests for `extractIdFromUrl()` function:
- Valid URLs with various ID formats
- Empty strings and null-like inputs
- URLs without IDs
- URLs with non-numeric content
- Edge cases: trailing slashes, whitespace, large numbers, overflow

### Episode Parsing Tests (26 tests)

Tests for `from_json(Episode)`:
- Valid episode JSON parsing
- Season/episode code extraction (S01E01 format)
- Character ID extraction from URLs
- Missing required fields
- Invalid field types
- Edge cases: empty arrays, special characters, Unicode

### Character Parsing Tests (50 tests)

Tests for `from_json(Character)`:
- Valid character JSON parsing
- Status mapping (Alive, Dead, Unknown)
- Gender mapping (Male, Female, Genderless, Unknown)
- Location reference parsing
- Episode ID extraction from URLs
- Missing required fields
- Invalid field types
- Edge cases: empty names, special characters, Unicode

## Test Fixtures

JSON fixtures in `tests/fixtures/json/` provide realistic test data:

### characters/single_character.json
```json
{
  "id": 1,
  "name": "Rick Sanchez",
  "status": "Alive",
  "species": "Human",
  "type": "",
  "gender": "Male",
  "origin": {
    "name": "Earth (C-137)",
    "url": "https://rickandmortyapi.com/api/location/1"
  },
  "location": {
    "name": "Citadel of Ricks",
    "url": "https://rickandmortyapi.com/api/location/3"
  },
  "image": "https://rickandmortyapi.com/api/character/avatar/1.jpeg",
  "episode": [
    "https://rickandmortyapi.com/api/episode/1",
    "https://rickandmortyapi.com/api/episode/2"
  ],
  "url": "https://rickandmortyapi.com/api/character/1",
  "created": "2017-11-04T18:48:46.250Z"
}
```

## Code Coverage

To generate code coverage reports:

```bash
# Configure with coverage flags
cmake -B .build/tests -DBUILD_TESTS=ON -DCMAKE_CXX_FLAGS="--coverage"

# Build and run tests
cmake --build .build/tests --target tests
cd .build/tests/tests-build && ctest

# Generate coverage report (requires lcov)
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## Continuous Integration

Tests run automatically on every push via GitHub Actions across all 4 platforms:

| Platform | Test Runner | Docker Image |
|----------|-------------|--------------|
| Linux x86_64 | Native Linux | `Dockerfile.test-runner` |
| Linux ARM64 | QEMU ARM64 emulation | `Dockerfile.test-runner` |
| Windows x86_64 | Wine 8.0+ | `Dockerfile.wine64` |
| Windows ARM64 | Wine 10.17 | `Dockerfile.wine10-arm64` |

### Running Tests Locally via Docker

```bash
# Build tests
docker compose run --rm test-build-linux-x86_64
docker compose run --rm test-build-windows-arm64

# Run tests
docker compose run --rm test-run-linux-x86_64
docker compose run --rm test-run-windows-arm64  # Uses Wine 10

# Test results are written to test-results/*.xml
```

### Wine 10 for ARM64

Windows ARM64 tests require Wine 10.2+ due to ARM64 compatibility fixes:
- Wine 8.0: Crashes with page fault in ucrtbase
- Wine 9.0: Hangs during wineboot initialization
- Wine 10.2+: Fixed ARM64 hang bug ([LP#1100695](https://bugs.launchpad.net/wine/+bug/1100695))

The `Dockerfile.wine10-arm64` builds Wine 10.17 from source (~7 min cached).

### CI Workflow Structure

The CI runs two independent execution branches in parallel:

```
x86_64 Branch: build-linux-x86_64 → build-windows-x86_64
ARM64 Branch:  build-linux-arm64  → build-windows-arm64
```

- Windows builds start as soon as their corresponding Linux build completes
- Each branch uses its own host Qt for cross-compilation
- All tests must pass on all 4 platforms
- Test failures block PR merges
- Test results are uploaded as JUnit XML artifacts

## Best Practices

1. **Test naming**: Use descriptive names that explain what's being tested
   - Good: `ThrowsOnMissingIdField`
   - Bad: `Test1`

2. **Arrange-Act-Assert**: Structure tests clearly
   ```cpp
   TEST_F(ParserTest, ParsesValidInput) {
       // Arrange
       json input = createValidJson();

       // Act
       auto result = parse(input);

       // Assert
       EXPECT_EQ(result.id, 1);
   }
   ```

3. **Test one thing**: Each test should verify a single behavior

4. **Use fixtures**: Store complex test data in JSON files

5. **Prefer fakes over mocks**: Use `FakeHttpClient` for simpler tests

6. **Document edge cases**: Comment tests that verify non-obvious behavior

## See Also

- [Google Test Documentation](https://google.github.io/googletest/)
- [Google Mock Documentation](https://google.github.io/googletest/gmock_for_dummies.html)
- [RapidCheck Documentation](https://github.com/emil-e/rapidcheck)
