#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace rickmorty {

enum class CharacterStatus {
    Alive,
    Dead,
    Unknown
};

enum class Gender {
    Female,
    Male,
    Genderless,
    Unknown
};

inline std::string statusToString(CharacterStatus status) {
    switch (status) {
        case CharacterStatus::Alive: return "Alive";
        case CharacterStatus::Dead: return "Dead";
        default: return "unknown";
    }
}

inline std::string genderToString(Gender gender) {
    switch (gender) {
        case Gender::Female: return "Female";
        case Gender::Male: return "Male";
        case Gender::Genderless: return "Genderless";
        default: return "unknown";
    }
}

struct LocationReference {
    std::string name;
    std::string url;
    int id = -1;
};

struct Character {
    int id = 0;
    std::string name;
    CharacterStatus status = CharacterStatus::Unknown;
    std::string species;
    std::string type;
    Gender gender = Gender::Unknown;
    LocationReference origin;
    LocationReference location;
    std::string imageUrl;
    std::vector<int> episodeIds;
    std::string url;
    std::string created;

    bool operator<(const Character& other) const {
        return name < other.name;
    }
};

struct Location {
    int id = 0;
    std::string name;
    std::string type;
    std::string dimension;
    std::vector<int> residentIds;
    std::string url;
    std::string created;
};

struct Episode {
    int id = 0;
    std::string name;
    std::string airDate;
    std::string episodeCode;
    std::vector<int> characterIds;
    std::string url;
    std::string created;
    int season = 0;
    int episodeNumber = 0;
};

struct PaginationInfo {
    int count = 0;
    int pages = 0;
    std::optional<std::string> next;
    std::optional<std::string> prev;
};

// Helper to extract ID from URL
inline int extractIdFromUrl(const std::string& url) {
    if (url.empty()) return -1;
    auto pos = url.rfind('/');
    if (pos != std::string::npos && pos + 1 < url.size()) {
        try {
            return std::stoi(url.substr(pos + 1));
        } catch (...) {
            return -1;
        }
    }
    return -1;
}

// JSON deserialization
inline void from_json(const nlohmann::json& j, LocationReference& loc) {
    j.at("name").get_to(loc.name);
    if (j.contains("url") && !j.at("url").is_null()) {
        loc.url = j.at("url").get<std::string>();
        loc.id = extractIdFromUrl(loc.url);
    }
}

inline void from_json(const nlohmann::json& j, Character& c) {
    j.at("id").get_to(c.id);
    j.at("name").get_to(c.name);
    j.at("species").get_to(c.species);
    j.at("type").get_to(c.type);
    j.at("image").get_to(c.imageUrl);
    j.at("url").get_to(c.url);
    j.at("created").get_to(c.created);

    std::string status = j.at("status").get<std::string>();
    if (status == "Alive") c.status = CharacterStatus::Alive;
    else if (status == "Dead") c.status = CharacterStatus::Dead;
    else c.status = CharacterStatus::Unknown;

    std::string gender = j.at("gender").get<std::string>();
    if (gender == "Female") c.gender = Gender::Female;
    else if (gender == "Male") c.gender = Gender::Male;
    else if (gender == "Genderless") c.gender = Gender::Genderless;
    else c.gender = Gender::Unknown;

    c.origin = j.at("origin").get<LocationReference>();
    c.location = j.at("location").get<LocationReference>();

    for (const auto& epUrl : j.at("episode")) {
        c.episodeIds.push_back(extractIdFromUrl(epUrl.get<std::string>()));
    }
}

inline void from_json(const nlohmann::json& j, Episode& e) {
    j.at("id").get_to(e.id);
    j.at("name").get_to(e.name);
    j.at("air_date").get_to(e.airDate);
    j.at("episode").get_to(e.episodeCode);
    j.at("url").get_to(e.url);
    j.at("created").get_to(e.created);

    // Parse episode code (S01E01 format)
    if (e.episodeCode.length() >= 6 && e.episodeCode[0] == 'S') {
        try {
            e.season = std::stoi(e.episodeCode.substr(1, 2));
            e.episodeNumber = std::stoi(e.episodeCode.substr(4, 2));
        } catch (...) {
            e.season = 0;
            e.episodeNumber = 0;
        }
    }

    for (const auto& charUrl : j.at("characters")) {
        e.characterIds.push_back(extractIdFromUrl(charUrl.get<std::string>()));
    }
}

inline void from_json(const nlohmann::json& j, Location& l) {
    j.at("id").get_to(l.id);
    j.at("name").get_to(l.name);
    j.at("type").get_to(l.type);
    j.at("dimension").get_to(l.dimension);
    j.at("url").get_to(l.url);
    j.at("created").get_to(l.created);

    for (const auto& resUrl : j.at("residents")) {
        l.residentIds.push_back(extractIdFromUrl(resUrl.get<std::string>()));
    }
}

inline void from_json(const nlohmann::json& j, PaginationInfo& p) {
    j.at("count").get_to(p.count);
    j.at("pages").get_to(p.pages);

    if (j.contains("next") && !j.at("next").is_null()) {
        p.next = j.at("next").get<std::string>();
    }
    if (j.contains("prev") && !j.at("prev").is_null()) {
        p.prev = j.at("prev").get<std::string>();
    }
}

} // namespace rickmorty
