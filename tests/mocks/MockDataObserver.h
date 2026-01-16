#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <vector>

#include "core/Observer.h"
#include "core/Models.h"

namespace rickmorty {
namespace testing {

/**
 * @brief GMock implementation of IDataObserver for behavioral verification.
 *
 * This mock allows verification of:
 * - Method calls and their arguments
 * - Call sequences and ordering
 * - Argument matching using custom matchers
 */
class MockDataObserver : public IDataObserver {
public:
    MockDataObserver() = default;
    ~MockDataObserver() override = default;

    // Mock all IDataObserver interface methods
    MOCK_METHOD(void, onEpisodesLoaded, (const std::vector<Episode>& episodes), (override));
    MOCK_METHOD(void, onCharactersLoaded, (int episodeId, const std::vector<Character>& characters), (override));
    MOCK_METHOD(void, onLoadingStateChanged, (bool isLoading), (override));
    MOCK_METHOD(void, onError, (const std::string& message), (override));
};

// =============================================================================
// Custom Matchers for Episode vectors
// =============================================================================

/**
 * @brief Matcher that checks if a vector of Episodes contains an episode with the specified ID.
 * @param id The episode ID to search for
 *
 * Usage:
 *   EXPECT_CALL(mock, onEpisodesLoaded(ContainsEpisodeWithId(42)));
 */
MATCHER_P(ContainsEpisodeWithId, id,
          std::string(negation ? "doesn't contain" : "contains") +
          " episode with id " + std::to_string(id)) {
    const auto& episodes = arg;
    return std::any_of(episodes.begin(), episodes.end(),
                       [id](const Episode& ep) { return ep.id == id; });
}

/**
 * @brief Matcher that checks if a vector of Episodes has exactly n elements.
 * @param n The expected count
 *
 * Usage:
 *   EXPECT_CALL(mock, onEpisodesLoaded(HasEpisodeCount(10)));
 */
MATCHER_P(HasEpisodeCount, n,
          std::string(negation ? "doesn't have" : "has") +
          " episode count of " + std::to_string(n)) {
    const auto& episodes = arg;
    return static_cast<int>(episodes.size()) == n;
}

/**
 * @brief Matcher that checks if a vector of Episodes is sorted by ID in ascending order.
 *
 * Usage:
 *   EXPECT_CALL(mock, onEpisodesLoaded(EpisodesAreSortedById()));
 */
MATCHER(EpisodesAreSortedById,
        std::string(negation ? "is not" : "is") + " sorted by id") {
    const auto& episodes = arg;
    if (episodes.size() <= 1) {
        return true;
    }
    return std::is_sorted(episodes.begin(), episodes.end(),
                          [](const Episode& a, const Episode& b) {
                              return a.id < b.id;
                          });
}

// =============================================================================
// Custom Matchers for Character vectors
// =============================================================================

/**
 * @brief Matcher that checks if a vector of Characters contains a character with the specified name.
 * @param name The character name to search for (case-sensitive)
 *
 * Usage:
 *   EXPECT_CALL(mock, onCharactersLoaded(_, ContainsCharacterNamed("Rick Sanchez")));
 */
MATCHER_P(ContainsCharacterNamed, name,
          std::string(negation ? "doesn't contain" : "contains") +
          " character named '" + std::string(name) + "'") {
    const auto& characters = arg;
    return std::any_of(characters.begin(), characters.end(),
                       [name](const Character& ch) { return ch.name == name; });
}

/**
 * @brief Matcher that checks if a vector of Characters has exactly n elements.
 * @param n The expected count
 *
 * Usage:
 *   EXPECT_CALL(mock, onCharactersLoaded(_, HasCharacterCount(5)));
 */
MATCHER_P(HasCharacterCount, n,
          std::string(negation ? "doesn't have" : "has") +
          " character count of " + std::to_string(n)) {
    const auto& characters = arg;
    return static_cast<int>(characters.size()) == n;
}

/**
 * @brief Matcher that checks if a vector of Characters is sorted by name in ascending order.
 *
 * Usage:
 *   EXPECT_CALL(mock, onCharactersLoaded(_, CharactersAreSortedByName()));
 */
MATCHER(CharactersAreSortedByName,
        std::string(negation ? "is not" : "is") + " sorted by name") {
    const auto& characters = arg;
    if (characters.size() <= 1) {
        return true;
    }
    return std::is_sorted(characters.begin(), characters.end(),
                          [](const Character& a, const Character& b) {
                              return a.name < b.name;
                          });
}

// =============================================================================
// Custom Matchers for Error messages
// =============================================================================

/**
 * @brief Matcher that checks if an error message contains the specified substring.
 * @param substring The substring to search for in the error message
 *
 * Usage:
 *   EXPECT_CALL(mock, onError(ErrorContains("network")));
 */
MATCHER_P(ErrorContains, substring,
          std::string(negation ? "doesn't contain" : "contains") +
          " substring '" + std::string(substring) + "'") {
    const std::string& message = arg;
    return message.find(substring) != std::string::npos;
}

// =============================================================================
// Additional Utility Matchers
// =============================================================================

/**
 * @brief Matcher that checks if a vector of Episodes contains an episode with the specified code.
 * @param code The episode code to search for (e.g., "S01E01")
 *
 * Usage:
 *   EXPECT_CALL(mock, onEpisodesLoaded(ContainsEpisodeWithCode("S01E01")));
 */
MATCHER_P(ContainsEpisodeWithCode, code,
          std::string(negation ? "doesn't contain" : "contains") +
          " episode with code '" + std::string(code) + "'") {
    const auto& episodes = arg;
    return std::any_of(episodes.begin(), episodes.end(),
                       [code](const Episode& ep) { return ep.episodeCode == code; });
}

/**
 * @brief Matcher that checks if a vector of Characters contains a character with the specified ID.
 * @param id The character ID to search for
 *
 * Usage:
 *   EXPECT_CALL(mock, onCharactersLoaded(_, ContainsCharacterWithId(1)));
 */
MATCHER_P(ContainsCharacterWithId, id,
          std::string(negation ? "doesn't contain" : "contains") +
          " character with id " + std::to_string(id)) {
    const auto& characters = arg;
    return std::any_of(characters.begin(), characters.end(),
                       [id](const Character& ch) { return ch.id == id; });
}

/**
 * @brief Matcher that checks if a vector of Characters contains a character with the specified status.
 * @param status The character status to search for
 *
 * Usage:
 *   EXPECT_CALL(mock, onCharactersLoaded(_, ContainsCharacterWithStatus(CharacterStatus::Alive)));
 */
MATCHER_P(ContainsCharacterWithStatus, status,
          std::string(negation ? "doesn't contain" : "contains") +
          " character with status " + statusToString(status)) {
    const auto& characters = arg;
    return std::any_of(characters.begin(), characters.end(),
                       [status](const Character& ch) { return ch.status == status; });
}

/**
 * @brief Matcher that checks if all episodes in the vector belong to the specified season.
 * @param season The season number to check
 *
 * Usage:
 *   EXPECT_CALL(mock, onEpisodesLoaded(AllEpisodesInSeason(1)));
 */
MATCHER_P(AllEpisodesInSeason, season,
          std::string(negation ? "not all episodes are" : "all episodes are") +
          " in season " + std::to_string(season)) {
    const auto& episodes = arg;
    if (episodes.empty()) {
        return !negation;  // Empty vector matches "all" in positive case
    }
    return std::all_of(episodes.begin(), episodes.end(),
                       [season](const Episode& ep) { return ep.season == season; });
}

/**
 * @brief Matcher that checks if the error message matches a specific category pattern.
 * Common patterns: "network", "parse", "timeout", "not found"
 * @param category The error category to check (checked case-insensitively)
 *
 * Usage:
 *   EXPECT_CALL(mock, onError(ErrorCategoryIs("network")));
 */
MATCHER_P(ErrorCategoryIs, category,
          std::string(negation ? "error is not" : "error is") +
          " category '" + std::string(category) + "'") {
    const std::string& message = arg;
    std::string lowerMessage = message;
    std::string lowerCategory = category;

    std::transform(lowerMessage.begin(), lowerMessage.end(), lowerMessage.begin(), ::tolower);
    std::transform(lowerCategory.begin(), lowerCategory.end(), lowerCategory.begin(), ::tolower);

    return lowerMessage.find(lowerCategory) != std::string::npos;
}

// =============================================================================
// Composite/Combining Matchers
// =============================================================================

/**
 * @brief Matcher that checks if episodes are sorted by ID and have expected count.
 * @param expectedCount The expected number of episodes
 *
 * Usage:
 *   EXPECT_CALL(mock, onEpisodesLoaded(EpisodesAreSortedWithCount(10)));
 */
MATCHER_P(EpisodesAreSortedWithCount, expectedCount,
          std::string(negation ? "is not" : "is") +
          " sorted by id with count " + std::to_string(expectedCount)) {
    const auto& episodes = arg;
    if (static_cast<int>(episodes.size()) != expectedCount) {
        return false;
    }
    if (episodes.size() <= 1) {
        return true;
    }
    return std::is_sorted(episodes.begin(), episodes.end(),
                          [](const Episode& a, const Episode& b) {
                              return a.id < b.id;
                          });
}

/**
 * @brief Matcher that checks if characters are sorted by name and have expected count.
 * @param expectedCount The expected number of characters
 *
 * Usage:
 *   EXPECT_CALL(mock, onCharactersLoaded(_, CharactersAreSortedWithCount(5)));
 */
MATCHER_P(CharactersAreSortedWithCount, expectedCount,
          std::string(negation ? "is not" : "is") +
          " sorted by name with count " + std::to_string(expectedCount)) {
    const auto& characters = arg;
    if (static_cast<int>(characters.size()) != expectedCount) {
        return false;
    }
    if (characters.size() <= 1) {
        return true;
    }
    return std::is_sorted(characters.begin(), characters.end(),
                          [](const Character& a, const Character& b) {
                              return a.name < b.name;
                          });
}

// =============================================================================
// Nice/Strict Mock Variants
// =============================================================================

/**
 * @brief NiceMock variant that ignores uninteresting calls.
 * Use when you only want to verify specific interactions.
 */
using NiceMockDataObserver = ::testing::NiceMock<MockDataObserver>;

/**
 * @brief StrictMock variant that fails on unexpected calls.
 * Use when you want to verify exact interaction sequences.
 */
using StrictMockDataObserver = ::testing::StrictMock<MockDataObserver>;

}  // namespace testing
}  // namespace rickmorty
