#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>
#include "core/Models.h"

namespace rickmorty {
namespace {

using json = nlohmann::json;
using ::testing::ElementsAre;
using ::testing::UnorderedElementsAre;

class CharacterParsingTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    // Helper to create a valid character JSON object
    json createValidCharacterJson() {
        return json{
            {"id", 1},
            {"name", "Rick Sanchez"},
            {"status", "Alive"},
            {"species", "Human"},
            {"type", ""},
            {"gender", "Male"},
            {"origin", {
                {"name", "Earth (C-137)"},
                {"url", "https://rickandmortyapi.com/api/location/1"}
            }},
            {"location", {
                {"name", "Citadel of Ricks"},
                {"url", "https://rickandmortyapi.com/api/location/3"}
            }},
            {"image", "https://rickandmortyapi.com/api/character/avatar/1.jpeg"},
            {"episode", json::array({
                "https://rickandmortyapi.com/api/episode/1",
                "https://rickandmortyapi.com/api/episode/2",
                "https://rickandmortyapi.com/api/episode/3"
            })},
            {"url", "https://rickandmortyapi.com/api/character/1"},
            {"created", "2017-11-04T18:48:46.250Z"}
        };
    }
};

// Test valid character JSON parses correctly

TEST_F(CharacterParsingTest, ParsesValidCharacterJson) {
    json j = createValidCharacterJson();

    Character character = j.get<Character>();

    EXPECT_EQ(character.id, 1);
    EXPECT_EQ(character.name, "Rick Sanchez");
    EXPECT_EQ(character.species, "Human");
    EXPECT_EQ(character.type, "");
    EXPECT_EQ(character.imageUrl, "https://rickandmortyapi.com/api/character/avatar/1.jpeg");
    EXPECT_EQ(character.url, "https://rickandmortyapi.com/api/character/1");
    EXPECT_EQ(character.created, "2017-11-04T18:48:46.250Z");
}

TEST_F(CharacterParsingTest, ParsesCharacterWithTypeField) {
    json j = createValidCharacterJson();
    j["type"] = "Genetic experiment";

    Character character = j.get<Character>();

    EXPECT_EQ(character.type, "Genetic experiment");
}

TEST_F(CharacterParsingTest, ParsesCharacterWithAllFields) {
    json j = {
        {"id", 2},
        {"name", "Morty Smith"},
        {"status", "Alive"},
        {"species", "Human"},
        {"type", ""},
        {"gender", "Male"},
        {"origin", {
            {"name", "unknown"},
            {"url", ""}
        }},
        {"location", {
            {"name", "Citadel of Ricks"},
            {"url", "https://rickandmortyapi.com/api/location/3"}
        }},
        {"image", "https://rickandmortyapi.com/api/character/avatar/2.jpeg"},
        {"episode", json::array({
            "https://rickandmortyapi.com/api/episode/1",
            "https://rickandmortyapi.com/api/episode/2"
        })},
        {"url", "https://rickandmortyapi.com/api/character/2"},
        {"created", "2017-11-04T18:50:21.651Z"}
    };

    Character character = j.get<Character>();

    EXPECT_EQ(character.id, 2);
    EXPECT_EQ(character.name, "Morty Smith");
}

// Test status enum mapping

TEST_F(CharacterParsingTest, MapsStatusAlive) {
    json j = createValidCharacterJson();
    j["status"] = "Alive";

    Character character = j.get<Character>();

    EXPECT_EQ(character.status, CharacterStatus::Alive);
}

TEST_F(CharacterParsingTest, MapsStatusDead) {
    json j = createValidCharacterJson();
    j["status"] = "Dead";

    Character character = j.get<Character>();

    EXPECT_EQ(character.status, CharacterStatus::Dead);
}

TEST_F(CharacterParsingTest, MapsStatusUnknown) {
    json j = createValidCharacterJson();
    j["status"] = "unknown";

    Character character = j.get<Character>();

    EXPECT_EQ(character.status, CharacterStatus::Unknown);
}

TEST_F(CharacterParsingTest, MapsUnrecognizedStatusToUnknown) {
    json j = createValidCharacterJson();
    j["status"] = "SomeOtherStatus";

    Character character = j.get<Character>();

    EXPECT_EQ(character.status, CharacterStatus::Unknown);
}

TEST_F(CharacterParsingTest, MapsEmptyStatusToUnknown) {
    json j = createValidCharacterJson();
    j["status"] = "";

    Character character = j.get<Character>();

    EXPECT_EQ(character.status, CharacterStatus::Unknown);
}

// Test statusToString helper

TEST_F(CharacterParsingTest, StatusToStringAlive) {
    EXPECT_EQ(statusToString(CharacterStatus::Alive), "Alive");
}

TEST_F(CharacterParsingTest, StatusToStringDead) {
    EXPECT_EQ(statusToString(CharacterStatus::Dead), "Dead");
}

TEST_F(CharacterParsingTest, StatusToStringUnknown) {
    EXPECT_EQ(statusToString(CharacterStatus::Unknown), "unknown");
}

// Test gender enum mapping

TEST_F(CharacterParsingTest, MapsGenderFemale) {
    json j = createValidCharacterJson();
    j["gender"] = "Female";

    Character character = j.get<Character>();

    EXPECT_EQ(character.gender, Gender::Female);
}

TEST_F(CharacterParsingTest, MapsGenderMale) {
    json j = createValidCharacterJson();
    j["gender"] = "Male";

    Character character = j.get<Character>();

    EXPECT_EQ(character.gender, Gender::Male);
}

TEST_F(CharacterParsingTest, MapsGenderGenderless) {
    json j = createValidCharacterJson();
    j["gender"] = "Genderless";

    Character character = j.get<Character>();

    EXPECT_EQ(character.gender, Gender::Genderless);
}

TEST_F(CharacterParsingTest, MapsGenderUnknown) {
    json j = createValidCharacterJson();
    j["gender"] = "unknown";

    Character character = j.get<Character>();

    EXPECT_EQ(character.gender, Gender::Unknown);
}

TEST_F(CharacterParsingTest, MapsUnrecognizedGenderToUnknown) {
    json j = createValidCharacterJson();
    j["gender"] = "NonBinary";

    Character character = j.get<Character>();

    EXPECT_EQ(character.gender, Gender::Unknown);
}

TEST_F(CharacterParsingTest, MapsEmptyGenderToUnknown) {
    json j = createValidCharacterJson();
    j["gender"] = "";

    Character character = j.get<Character>();

    EXPECT_EQ(character.gender, Gender::Unknown);
}

// Test genderToString helper

TEST_F(CharacterParsingTest, GenderToStringFemale) {
    EXPECT_EQ(genderToString(Gender::Female), "Female");
}

TEST_F(CharacterParsingTest, GenderToStringMale) {
    EXPECT_EQ(genderToString(Gender::Male), "Male");
}

TEST_F(CharacterParsingTest, GenderToStringGenderless) {
    EXPECT_EQ(genderToString(Gender::Genderless), "Genderless");
}

TEST_F(CharacterParsingTest, GenderToStringUnknown) {
    EXPECT_EQ(genderToString(Gender::Unknown), "unknown");
}

// Test location reference parsing

TEST_F(CharacterParsingTest, ParsesOriginLocationReference) {
    json j = createValidCharacterJson();

    Character character = j.get<Character>();

    EXPECT_EQ(character.origin.name, "Earth (C-137)");
    EXPECT_EQ(character.origin.url, "https://rickandmortyapi.com/api/location/1");
    EXPECT_EQ(character.origin.id, 1);
}

TEST_F(CharacterParsingTest, ParsesCurrentLocationReference) {
    json j = createValidCharacterJson();

    Character character = j.get<Character>();

    EXPECT_EQ(character.location.name, "Citadel of Ricks");
    EXPECT_EQ(character.location.url, "https://rickandmortyapi.com/api/location/3");
    EXPECT_EQ(character.location.id, 3);
}

TEST_F(CharacterParsingTest, ParsesLocationWithEmptyUrl) {
    json j = createValidCharacterJson();
    j["origin"]["url"] = "";

    Character character = j.get<Character>();

    EXPECT_EQ(character.origin.name, "Earth (C-137)");
    EXPECT_EQ(character.origin.url, "");
    EXPECT_EQ(character.origin.id, -1);
}

TEST_F(CharacterParsingTest, ParsesLocationWithUnknownName) {
    json j = createValidCharacterJson();
    j["origin"]["name"] = "unknown";
    j["origin"]["url"] = "";

    Character character = j.get<Character>();

    EXPECT_EQ(character.origin.name, "unknown");
    EXPECT_EQ(character.origin.id, -1);
}

TEST_F(CharacterParsingTest, ParsesLocationWithLargeId) {
    json j = createValidCharacterJson();
    j["location"]["url"] = "https://rickandmortyapi.com/api/location/126";

    Character character = j.get<Character>();

    EXPECT_EQ(character.location.id, 126);
}

// Test episode IDs extracted from URLs

TEST_F(CharacterParsingTest, ExtractsEpisodeIdsFromUrls) {
    json j = createValidCharacterJson();

    Character character = j.get<Character>();

    EXPECT_THAT(character.episodeIds, ElementsAre(1, 2, 3));
}

TEST_F(CharacterParsingTest, ExtractsMultipleEpisodeIds) {
    json j = createValidCharacterJson();
    j["episode"] = json::array({
        "https://rickandmortyapi.com/api/episode/1",
        "https://rickandmortyapi.com/api/episode/2",
        "https://rickandmortyapi.com/api/episode/3",
        "https://rickandmortyapi.com/api/episode/4",
        "https://rickandmortyapi.com/api/episode/5",
        "https://rickandmortyapi.com/api/episode/6",
        "https://rickandmortyapi.com/api/episode/7",
        "https://rickandmortyapi.com/api/episode/8",
        "https://rickandmortyapi.com/api/episode/9",
        "https://rickandmortyapi.com/api/episode/10",
        "https://rickandmortyapi.com/api/episode/11"
    });

    Character character = j.get<Character>();

    EXPECT_THAT(character.episodeIds, ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11));
}

TEST_F(CharacterParsingTest, HandlesEmptyEpisodeList) {
    json j = createValidCharacterJson();
    j["episode"] = json::array();

    Character character = j.get<Character>();

    EXPECT_TRUE(character.episodeIds.empty());
}

TEST_F(CharacterParsingTest, HandlesSingleEpisode) {
    json j = createValidCharacterJson();
    j["episode"] = json::array({
        "https://rickandmortyapi.com/api/episode/10"
    });

    Character character = j.get<Character>();

    EXPECT_THAT(character.episodeIds, ElementsAre(10));
}

TEST_F(CharacterParsingTest, HandlesLargeEpisodeIds) {
    json j = createValidCharacterJson();
    j["episode"] = json::array({
        "https://rickandmortyapi.com/api/episode/51",
        "https://rickandmortyapi.com/api/episode/100"
    });

    Character character = j.get<Character>();

    EXPECT_THAT(character.episodeIds, ElementsAre(51, 100));
}

// Test missing required fields throw

TEST_F(CharacterParsingTest, ThrowsOnMissingId) {
    json j = createValidCharacterJson();
    j.erase("id");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingName) {
    json j = createValidCharacterJson();
    j.erase("name");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingStatus) {
    json j = createValidCharacterJson();
    j.erase("status");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingSpecies) {
    json j = createValidCharacterJson();
    j.erase("species");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingType) {
    json j = createValidCharacterJson();
    j.erase("type");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingGender) {
    json j = createValidCharacterJson();
    j.erase("gender");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingOrigin) {
    json j = createValidCharacterJson();
    j.erase("origin");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingLocation) {
    json j = createValidCharacterJson();
    j.erase("location");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingImage) {
    json j = createValidCharacterJson();
    j.erase("image");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingEpisode) {
    json j = createValidCharacterJson();
    j.erase("episode");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingUrl) {
    json j = createValidCharacterJson();
    j.erase("url");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnMissingCreated) {
    json j = createValidCharacterJson();
    j.erase("created");

    EXPECT_THROW(j.get<Character>(), json::out_of_range);
}

TEST_F(CharacterParsingTest, ThrowsOnWrongIdType) {
    json j = createValidCharacterJson();
    j["id"] = "not_a_number";

    EXPECT_THROW(j.get<Character>(), json::type_error);
}

TEST_F(CharacterParsingTest, HandlesWrongEpisodeType) {
    // Note: When episode is a string, nlohmann::json treats it as a single element
    // extractIdFromUrl("not_an_array") returns -1
    json j = createValidCharacterJson();
    j["episode"] = "not_an_array";

    Character character = j.get<Character>();
    EXPECT_EQ(character.episodeIds.size(), 1u);
    EXPECT_EQ(character.episodeIds[0], -1);
}

// Additional edge case tests

TEST_F(CharacterParsingTest, ParsesCharacterWithEmptyName) {
    json j = createValidCharacterJson();
    j["name"] = "";

    Character character = j.get<Character>();

    EXPECT_EQ(character.name, "");
}

TEST_F(CharacterParsingTest, ParsesCharacterWithSpecialCharactersInName) {
    json j = createValidCharacterJson();
    j["name"] = "Rick 'The Genius' Sanchez (C-137)";

    Character character = j.get<Character>();

    EXPECT_EQ(character.name, "Rick 'The Genius' Sanchez (C-137)");
}

TEST_F(CharacterParsingTest, ParsesCharacterWithUnicodeInName) {
    json j = createValidCharacterJson();
    j["name"] = "Fran\u00E7ois Le Morty";

    Character character = j.get<Character>();

    EXPECT_EQ(character.name, "Fran\u00E7ois Le Morty");
}

TEST_F(CharacterParsingTest, CharacterLessThanOperatorComparesByName) {
    json j1 = createValidCharacterJson();
    j1["name"] = "Alpha";
    j1["id"] = 100;

    json j2 = createValidCharacterJson();
    j2["name"] = "Beta";
    j2["id"] = 1;

    Character c1 = j1.get<Character>();
    Character c2 = j2.get<Character>();

    EXPECT_TRUE(c1 < c2);
    EXPECT_FALSE(c2 < c1);
}

TEST_F(CharacterParsingTest, CharacterLessThanOperatorEqualNames) {
    json j1 = createValidCharacterJson();
    json j2 = createValidCharacterJson();

    Character c1 = j1.get<Character>();
    Character c2 = j2.get<Character>();

    EXPECT_FALSE(c1 < c2);
    EXPECT_FALSE(c2 < c1);
}

}  // namespace
}  // namespace rickmorty
