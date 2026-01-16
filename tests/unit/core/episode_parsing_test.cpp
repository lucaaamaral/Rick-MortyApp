#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include "core/Models.h"

namespace rickmorty {
namespace {

using json = nlohmann::json;
using ::testing::ElementsAre;
using ::testing::UnorderedElementsAre;

class EpisodeParsingTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    // Helper to create a valid episode JSON object
    json createValidEpisodeJson() {
        return json{
            {"id", 1},
            {"name", "Pilot"},
            {"air_date", "December 2, 2013"},
            {"episode", "S01E01"},
            {"characters", json::array({
                "https://rickandmortyapi.com/api/character/1",
                "https://rickandmortyapi.com/api/character/2",
                "https://rickandmortyapi.com/api/character/35"
            })},
            {"url", "https://rickandmortyapi.com/api/episode/1"},
            {"created", "2017-11-10T12:56:33.798Z"}
        };
    }
};

// Test valid episode JSON parses correctly

TEST_F(EpisodeParsingTest, ParsesValidEpisodeJson) {
    json j = createValidEpisodeJson();

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.id, 1);
    EXPECT_EQ(episode.name, "Pilot");
    EXPECT_EQ(episode.airDate, "December 2, 2013");
    EXPECT_EQ(episode.episodeCode, "S01E01");
    EXPECT_EQ(episode.url, "https://rickandmortyapi.com/api/episode/1");
    EXPECT_EQ(episode.created, "2017-11-10T12:56:33.798Z");
}

TEST_F(EpisodeParsingTest, ParsesEpisodeWithAllFields) {
    json j = {
        {"id", 28},
        {"name", "The Ricklantis Mixup"},
        {"air_date", "September 10, 2017"},
        {"episode", "S03E07"},
        {"characters", json::array({
            "https://rickandmortyapi.com/api/character/1",
            "https://rickandmortyapi.com/api/character/2"
        })},
        {"url", "https://rickandmortyapi.com/api/episode/28"},
        {"created", "2017-11-10T12:56:36.618Z"}
    };

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.id, 28);
    EXPECT_EQ(episode.name, "The Ricklantis Mixup");
    EXPECT_EQ(episode.season, 3);
    EXPECT_EQ(episode.episodeNumber, 7);
}

// Test season and episode number extraction from code

TEST_F(EpisodeParsingTest, ExtractsSeasonAndEpisodeNumberFromCode) {
    json j = createValidEpisodeJson();
    j["episode"] = "S01E01";

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.season, 1);
    EXPECT_EQ(episode.episodeNumber, 1);
}

TEST_F(EpisodeParsingTest, ExtractsDoubleDigitSeasonAndEpisode) {
    json j = createValidEpisodeJson();
    j["episode"] = "S10E25";

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.season, 10);
    EXPECT_EQ(episode.episodeNumber, 25);
}

TEST_F(EpisodeParsingTest, ExtractsSeasonFiveEpisodeThree) {
    json j = createValidEpisodeJson();
    j["episode"] = "S05E03";

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.season, 5);
    EXPECT_EQ(episode.episodeNumber, 3);
}

TEST_F(EpisodeParsingTest, HandlesInvalidEpisodeCodeFormat) {
    json j = createValidEpisodeJson();
    j["episode"] = "INVALID";

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.season, 0);
    EXPECT_EQ(episode.episodeNumber, 0);
}

TEST_F(EpisodeParsingTest, HandlesEmptyEpisodeCode) {
    json j = createValidEpisodeJson();
    j["episode"] = "";

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.season, 0);
    EXPECT_EQ(episode.episodeNumber, 0);
}

TEST_F(EpisodeParsingTest, HandlesShortEpisodeCode) {
    json j = createValidEpisodeJson();
    j["episode"] = "S01";

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.season, 0);
    EXPECT_EQ(episode.episodeNumber, 0);
}

TEST_F(EpisodeParsingTest, HandlesEpisodeCodeNotStartingWithS) {
    json j = createValidEpisodeJson();
    j["episode"] = "E01S01";

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.season, 0);
    EXPECT_EQ(episode.episodeNumber, 0);
}

// Test character IDs extracted from URLs

TEST_F(EpisodeParsingTest, ExtractsCharacterIdsFromUrls) {
    json j = createValidEpisodeJson();

    Episode episode = j.get<Episode>();

    EXPECT_THAT(episode.characterIds, ElementsAre(1, 2, 35));
}

TEST_F(EpisodeParsingTest, ExtractsMultipleCharacterIds) {
    json j = createValidEpisodeJson();
    j["characters"] = json::array({
        "https://rickandmortyapi.com/api/character/1",
        "https://rickandmortyapi.com/api/character/2",
        "https://rickandmortyapi.com/api/character/35",
        "https://rickandmortyapi.com/api/character/38",
        "https://rickandmortyapi.com/api/character/62",
        "https://rickandmortyapi.com/api/character/92",
        "https://rickandmortyapi.com/api/character/127"
    });

    Episode episode = j.get<Episode>();

    EXPECT_THAT(episode.characterIds, ElementsAre(1, 2, 35, 38, 62, 92, 127));
}

TEST_F(EpisodeParsingTest, HandlesEmptyCharacterList) {
    json j = createValidEpisodeJson();
    j["characters"] = json::array();

    Episode episode = j.get<Episode>();

    EXPECT_TRUE(episode.characterIds.empty());
}

TEST_F(EpisodeParsingTest, HandlesSingleCharacter) {
    json j = createValidEpisodeJson();
    j["characters"] = json::array({
        "https://rickandmortyapi.com/api/character/42"
    });

    Episode episode = j.get<Episode>();

    EXPECT_THAT(episode.characterIds, ElementsAre(42));
}

TEST_F(EpisodeParsingTest, HandlesLargeCharacterIds) {
    json j = createValidEpisodeJson();
    j["characters"] = json::array({
        "https://rickandmortyapi.com/api/character/826",
        "https://rickandmortyapi.com/api/character/999"
    });

    Episode episode = j.get<Episode>();

    EXPECT_THAT(episode.characterIds, ElementsAre(826, 999));
}

// Test missing required fields throw

TEST_F(EpisodeParsingTest, ThrowsOnMissingId) {
    json j = createValidEpisodeJson();
    j.erase("id");

    EXPECT_THROW(j.get<Episode>(), json::out_of_range);
}

TEST_F(EpisodeParsingTest, ThrowsOnMissingName) {
    json j = createValidEpisodeJson();
    j.erase("name");

    EXPECT_THROW(j.get<Episode>(), json::out_of_range);
}

TEST_F(EpisodeParsingTest, ThrowsOnMissingAirDate) {
    json j = createValidEpisodeJson();
    j.erase("air_date");

    EXPECT_THROW(j.get<Episode>(), json::out_of_range);
}

TEST_F(EpisodeParsingTest, ThrowsOnMissingEpisodeCode) {
    json j = createValidEpisodeJson();
    j.erase("episode");

    EXPECT_THROW(j.get<Episode>(), json::out_of_range);
}

TEST_F(EpisodeParsingTest, ThrowsOnMissingCharacters) {
    json j = createValidEpisodeJson();
    j.erase("characters");

    EXPECT_THROW(j.get<Episode>(), json::out_of_range);
}

TEST_F(EpisodeParsingTest, ThrowsOnMissingUrl) {
    json j = createValidEpisodeJson();
    j.erase("url");

    EXPECT_THROW(j.get<Episode>(), json::out_of_range);
}

TEST_F(EpisodeParsingTest, ThrowsOnMissingCreated) {
    json j = createValidEpisodeJson();
    j.erase("created");

    EXPECT_THROW(j.get<Episode>(), json::out_of_range);
}

TEST_F(EpisodeParsingTest, ThrowsOnWrongIdType) {
    json j = createValidEpisodeJson();
    j["id"] = "not_a_number";

    EXPECT_THROW(j.get<Episode>(), json::type_error);
}

TEST_F(EpisodeParsingTest, HandlesWrongCharactersType) {
    // Note: When characters is a string, nlohmann::json treats it as a single element
    // extractIdFromUrl("not_an_array") returns -1
    json j = createValidEpisodeJson();
    j["characters"] = "not_an_array";

    Episode episode = j.get<Episode>();
    EXPECT_EQ(episode.characterIds.size(), 1u);
    EXPECT_EQ(episode.characterIds[0], -1);
}

// Additional edge case tests

TEST_F(EpisodeParsingTest, ParsesEpisodeWithEmptyName) {
    json j = createValidEpisodeJson();
    j["name"] = "";

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.name, "");
}

TEST_F(EpisodeParsingTest, ParsesEpisodeWithSpecialCharactersInName) {
    json j = createValidEpisodeJson();
    j["name"] = "Rick & Morty: The \"Special\" Episode!";

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.name, "Rick & Morty: The \"Special\" Episode!");
}

TEST_F(EpisodeParsingTest, ParsesEpisodeWithUnicodeInName) {
    json j = createValidEpisodeJson();
    j["name"] = "L'Episode Fran\u00E7ais";

    Episode episode = j.get<Episode>();

    EXPECT_EQ(episode.name, "L'Episode Fran\u00E7ais");
}

}  // namespace
}  // namespace rickmorty
