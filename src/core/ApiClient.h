#pragma once

#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <memory>
#include "Models.h"
#include "IHttpClient.h"

namespace rickmorty {

/**
 * @class ApiException
 * @brief Exception class for API-level errors.
 *
 * This exception is thrown when API operations fail, including network errors,
 * JSON parsing errors, and resource not found conditions.
 */
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

/**
 * @class ApiClient
 * @brief Client for the Rick and Morty API.
 *
 * This class provides methods to fetch data from the Rick and Morty API.
 * It supports dependency injection of an IHttpClient for testing purposes,
 * or can create a default CurlHttpClient for production use.
 *
 * Example usage with default HTTP client:
 * @code
 * ApiClient client;
 * auto episodes = client.fetchAllEpisodes();
 * @endcode
 *
 * Example usage with injected HTTP client (for testing):
 * @code
 * auto mockHttp = std::make_unique<MockHttpClient>();
 * ApiClient client(std::move(mockHttp));
 * @endcode
 */
class ApiClient {
public:
    /**
     * @brief Default constructor that creates a CurlHttpClient internally.
     * @throws ApiException if HTTP client initialization fails.
     */
    ApiClient();

    /**
     * @brief Constructor with dependency injection for the HTTP client.
     * @param httpClient The HTTP client to use for requests.
     *
     * This constructor enables unit testing by allowing injection of mock
     * HTTP clients that return predetermined responses.
     */
    explicit ApiClient(std::unique_ptr<IHttpClient> httpClient);

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

    template<typename T>
    std::vector<T> fetchAllPaginated(const std::string& endpoint);

    std::unique_ptr<IHttpClient> httpClient_;
};

} // namespace rickmorty
