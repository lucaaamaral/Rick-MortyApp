#pragma once

#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <curl/curl.h>
#include "Models.h"

namespace rickmorty {

class ApiException : public std::runtime_error {
public:
    enum class Type {
        NetworkError,
        ParseError,
        NotFound,
        Unknown
    };

    ApiException(Type type, const std::string& message)
        : std::runtime_error(message), type_(type) {}

    Type type() const { return type_; }

private:
    Type type_;
};

class ApiClient {
public:
    ApiClient();
    ~ApiClient();

    ApiClient(const ApiClient&) = delete;
    ApiClient& operator=(const ApiClient&) = delete;

    std::vector<Episode> fetchAllEpisodes();
    std::optional<Episode> fetchEpisode(int id);

    std::vector<Character> fetchCharacters(const std::vector<int>& ids);
    std::optional<Character> fetchCharacter(int id);

    std::optional<Location> fetchLocation(int id);

private:
    static constexpr const char* BASE_URL = "https://rickandmortyapi.com/api";

    std::string httpGet(const std::string& url);

    template<typename T>
    std::vector<T> fetchAllPaginated(const std::string& endpoint);

    CURL* curl_;
};

} // namespace rickmorty
