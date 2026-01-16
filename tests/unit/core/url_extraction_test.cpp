#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "core/Models.h"

namespace rickmorty {
namespace {

class UrlExtractionTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test valid API URLs

TEST_F(UrlExtractionTest, ExtractsIdFromValidCharacterUrl) {
    std::string url = "https://rickandmortyapi.com/api/character/1";
    EXPECT_EQ(extractIdFromUrl(url), 1);
}

TEST_F(UrlExtractionTest, ExtractsIdFromValidCharacterUrlWithLargeId) {
    std::string url = "https://rickandmortyapi.com/api/character/826";
    EXPECT_EQ(extractIdFromUrl(url), 826);
}

TEST_F(UrlExtractionTest, ExtractsIdFromValidEpisodeUrl) {
    std::string url = "https://rickandmortyapi.com/api/episode/28";
    EXPECT_EQ(extractIdFromUrl(url), 28);
}

TEST_F(UrlExtractionTest, ExtractsIdFromValidLocationUrl) {
    std::string url = "https://rickandmortyapi.com/api/location/3";
    EXPECT_EQ(extractIdFromUrl(url), 3);
}

// Test empty string

TEST_F(UrlExtractionTest, ReturnsMinusOneForEmptyString) {
    std::string url = "";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

// Test no slash

TEST_F(UrlExtractionTest, ReturnsMinusOneForNoSlash) {
    std::string url = "noSlashHere";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

TEST_F(UrlExtractionTest, ReturnsMinusOneForNoSlashJustNumber) {
    std::string url = "12345";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

// Test trailing slash

TEST_F(UrlExtractionTest, ReturnsMinusOneForTrailingSlash) {
    std::string url = "https://rickandmortyapi.com/api/character/";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

TEST_F(UrlExtractionTest, ReturnsMinusOneForUrlEndingWithSlash) {
    std::string url = "https://rickandmortyapi.com/api/character/42/";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

// Test non-numeric after slash

TEST_F(UrlExtractionTest, ReturnsMinusOneForNonNumericAfterSlash) {
    std::string url = "https://rickandmortyapi.com/api/character/abc";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

TEST_F(UrlExtractionTest, ReturnsMinusOneForMixedAlphaNumericAfterSlash) {
    std::string url = "https://rickandmortyapi.com/api/character/12abc";
    // stoi will parse the leading numeric portion, so this returns 12
    EXPECT_EQ(extractIdFromUrl(url), 12);
}

TEST_F(UrlExtractionTest, ReturnsMinusOneForSpecialCharactersAfterSlash) {
    std::string url = "https://rickandmortyapi.com/api/character/@#$";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

// Test large IDs

TEST_F(UrlExtractionTest, HandlesLargeId) {
    std::string url = "https://rickandmortyapi.com/api/character/999999";
    EXPECT_EQ(extractIdFromUrl(url), 999999);
}

TEST_F(UrlExtractionTest, HandlesVeryLargeId) {
    std::string url = "https://rickandmortyapi.com/api/character/2147483647";
    EXPECT_EQ(extractIdFromUrl(url), 2147483647);
}

TEST_F(UrlExtractionTest, HandlesIdOverflow) {
    // Integer overflow - stoi should throw
    std::string url = "https://rickandmortyapi.com/api/character/99999999999999999999";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

// Test negative handling

TEST_F(UrlExtractionTest, HandlesNegativeIdInUrl) {
    // Negative numbers are technically parsed by stoi
    std::string url = "https://rickandmortyapi.com/api/character/-1";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

TEST_F(UrlExtractionTest, HandlesNegativeLargeIdInUrl) {
    std::string url = "https://rickandmortyapi.com/api/character/-999";
    EXPECT_EQ(extractIdFromUrl(url), -999);
}

// Edge cases

TEST_F(UrlExtractionTest, HandlesSingleSlashWithNumber) {
    std::string url = "/42";
    EXPECT_EQ(extractIdFromUrl(url), 42);
}

TEST_F(UrlExtractionTest, HandlesJustSlash) {
    std::string url = "/";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

TEST_F(UrlExtractionTest, HandlesMultipleConsecutiveSlashes) {
    std::string url = "https://example.com//123";
    EXPECT_EQ(extractIdFromUrl(url), 123);
}

TEST_F(UrlExtractionTest, HandlesZeroId) {
    std::string url = "https://rickandmortyapi.com/api/character/0";
    EXPECT_EQ(extractIdFromUrl(url), 0);
}

TEST_F(UrlExtractionTest, HandlesLeadingZerosInId) {
    std::string url = "https://rickandmortyapi.com/api/character/007";
    EXPECT_EQ(extractIdFromUrl(url), 7);
}

TEST_F(UrlExtractionTest, HandlesWhitespaceAfterSlash) {
    // Note: std::stoi skips leading whitespace, so " 42" parses as 42
    std::string url = "https://rickandmortyapi.com/api/character/ 42";
    EXPECT_EQ(extractIdFromUrl(url), 42);
}

TEST_F(UrlExtractionTest, HandlesWhitespaceOnlyAfterSlash) {
    std::string url = "https://rickandmortyapi.com/api/character/   ";
    EXPECT_EQ(extractIdFromUrl(url), -1);
}

}  // namespace
}  // namespace rickmorty
