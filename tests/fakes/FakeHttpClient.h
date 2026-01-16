#pragma once

/**
 * @file FakeHttpClient.h
 * @brief Fake HTTP client for testing - returns fixture data without network calls.
 *
 * This fake implementation of IHttpClient provides a lightweight test double
 * that can be configured to return specific responses for URL patterns,
 * load responses from fixture files, and simulate various error conditions.
 */

#include "core/IHttpClient.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <optional>
#include <regex>
#include <memory>
#include <fstream>
#include <sstream>

namespace rickmorty {
namespace testing {

/**
 * @class FakeHttpClient
 * @brief A configurable fake HTTP client for unit testing.
 *
 * FakeHttpClient implements the IHttpClient interface and provides a fluent API
 * for configuring responses to URL patterns. It supports:
 * - Exact URL matching with static responses
 * - Regex URL pattern matching
 * - Loading responses from fixture files
 * - Simulating HTTP errors and network failures
 * - Tracking all requested URLs for test verification
 *
 * All operations are thread-safe.
 *
 * Example usage:
 * @code
 * FakeHttpClient fake;
 * fake.route("https://api.example.com/data", R"({"result": "success"})")
 *     .routeToFixture("https://api.example.com/characters", "fixtures/characters.json")
 *     .simulateError(HttpException::Type::NetworkError, "Connection refused");
 *
 * // Use in tests
 * auto response = fake.get("https://api.example.com/data");
 * EXPECT_EQ(fake.requestedUrls().size(), 1);
 * @endcode
 */
class FakeHttpClient : public IHttpClient {
public:
    /**
     * @brief Default constructor creates an empty fake client.
     */
    FakeHttpClient() = default;

    /**
     * @brief Virtual destructor.
     */
    ~FakeHttpClient() override = default;

    // Non-copyable, non-movable (due to mutex member)
    FakeHttpClient(const FakeHttpClient&) = delete;
    FakeHttpClient& operator=(const FakeHttpClient&) = delete;
    FakeHttpClient(FakeHttpClient&&) = delete;
    FakeHttpClient& operator=(FakeHttpClient&&) = delete;

    //=========================================================================
    // IHttpClient interface implementation
    //=========================================================================

    /**
     * @brief Performs a fake HTTP GET request.
     *
     * Looks up the URL in configured routes and returns the appropriate response
     * or throws an exception if configured to simulate an error.
     *
     * @param url The URL to request.
     * @return The configured response string.
     * @throws HttpException if configured to simulate an error, or if no route matches.
     */
    std::string get(const std::string& url) override;

    /**
     * @brief Sets the timeout (no-op for fake client, but records the value).
     * @param timeoutMs Timeout in milliseconds.
     */
    void setTimeout(long timeoutMs) override;

    /**
     * @brief Sets the user agent (no-op for fake client, but records the value).
     * @param userAgent The user agent string.
     */
    void setUserAgent(const std::string& userAgent) override;

    //=========================================================================
    // Fluent configuration API
    //=========================================================================

    /**
     * @brief Configures a static response for an exact URL match.
     *
     * @param url The exact URL to match.
     * @param response The response body to return.
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& route(const std::string& url, const std::string& response);

    /**
     * @brief Configures a response from a fixture file for an exact URL match.
     *
     * The fixture file is read when get() is called, not when this method is called,
     * allowing the fixture to be modified between calls if needed.
     *
     * @param url The exact URL to match.
     * @param fixturePath Path to the fixture file (relative to current directory or absolute).
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& routeToFixture(const std::string& url, const std::string& fixturePath);

    /**
     * @brief Configures a static response for URLs matching a regex pattern.
     *
     * Patterns are checked after exact matches, in the order they were added.
     *
     * @param pattern The regex pattern to match against URLs.
     * @param response The response body to return.
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& routePattern(const std::string& pattern, const std::string& response);

    /**
     * @brief Configures a fixture file response for URLs matching a regex pattern.
     *
     * @param pattern The regex pattern to match against URLs.
     * @param fixturePath Path to the fixture file.
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& routePatternToFixture(const std::string& pattern, const std::string& fixturePath);

    /**
     * @brief Configures a callback function to generate responses for a URL pattern.
     *
     * The callback receives the full URL and should return the response body.
     * This is useful for dynamic responses based on URL parameters.
     *
     * @param pattern The regex pattern to match against URLs.
     * @param handler Function that takes the URL and returns the response.
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& routePatternWithHandler(
        const std::string& pattern,
        std::function<std::string(const std::string&)> handler);

    /**
     * @brief Configures the client to simulate an error for all requests.
     *
     * This takes precedence over all route configurations.
     *
     * @param type The type of HTTP error to simulate.
     * @param message The error message.
     * @param httpCode Optional HTTP status code (default 0).
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& simulateError(
        HttpException::Type type,
        const std::string& message,
        int httpCode = 0);

    /**
     * @brief Configures the client to simulate an error for a specific URL.
     *
     * @param url The exact URL to fail.
     * @param type The type of HTTP error to simulate.
     * @param message The error message.
     * @param httpCode Optional HTTP status code (default 0).
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& simulateErrorForUrl(
        const std::string& url,
        HttpException::Type type,
        const std::string& message,
        int httpCode = 0);

    /**
     * @brief Clears the global error simulation.
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& clearGlobalError();

    /**
     * @brief Clears all configured routes and errors.
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& reset();

    /**
     * @brief Sets the base path for fixture file resolution.
     *
     * When set, fixture paths are resolved relative to this base path.
     *
     * @param basePath The base directory for fixture files.
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& setFixtureBasePath(const std::string& basePath);

    /**
     * @brief Sets a default response for unmatched URLs.
     *
     * If set, unmatched URLs return this response instead of throwing.
     *
     * @param response The default response body.
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& setDefaultResponse(const std::string& response);

    /**
     * @brief Clears the default response, causing unmatched URLs to throw.
     * @return Reference to this for method chaining.
     */
    FakeHttpClient& clearDefaultResponse();

    //=========================================================================
    // Verification API
    //=========================================================================

    /**
     * @brief Returns all URLs that have been requested.
     * @return Vector of requested URLs in order.
     */
    std::vector<std::string> requestedUrls() const;

    /**
     * @brief Returns the number of times a specific URL was requested.
     * @param url The URL to check.
     * @return Number of requests to that URL.
     */
    size_t requestCount(const std::string& url) const;

    /**
     * @brief Returns the total number of requests made.
     * @return Total request count.
     */
    size_t totalRequestCount() const;

    /**
     * @brief Checks if a URL was ever requested.
     * @param url The URL to check.
     * @return True if the URL was requested at least once.
     */
    bool wasRequested(const std::string& url) const;

    /**
     * @brief Checks if any URL matching a pattern was requested.
     * @param pattern Regex pattern to match against requested URLs.
     * @return True if any matching URL was requested.
     */
    bool wasRequestedMatching(const std::string& pattern) const;

    /**
     * @brief Clears the request history.
     */
    void clearRequestHistory();

    /**
     * @brief Returns the configured timeout value.
     * @return The timeout in milliseconds, or nullopt if not set.
     */
    std::optional<long> configuredTimeout() const;

    /**
     * @brief Returns the configured user agent.
     * @return The user agent string, or nullopt if not set.
     */
    std::optional<std::string> configuredUserAgent() const;

private:
    /**
     * @struct ErrorConfig
     * @brief Configuration for simulated errors.
     */
    struct ErrorConfig {
        HttpException::Type type;
        std::string message;
        int httpCode;
    };

    /**
     * @struct RouteConfig
     * @brief Configuration for a URL route.
     */
    struct RouteConfig {
        enum class Type {
            StaticResponse,
            FixtureFile,
            Handler
        };

        Type type;
        std::string response;        // For StaticResponse
        std::string fixturePath;     // For FixtureFile
        std::function<std::string(const std::string&)> handler; // For Handler
    };

    /**
     * @struct PatternRoute
     * @brief A route with regex pattern matching.
     */
    struct PatternRoute {
        std::regex pattern;
        std::string patternString;  // Original pattern for debugging
        RouteConfig config;
    };

    // Helper methods
    std::string loadFixture(const std::string& path) const;
    std::string getRouteResponse(const RouteConfig& config, const std::string& url) const;
    void recordRequest(const std::string& url);

    // Thread-safe access
    mutable std::mutex mutex_;

    // Route configurations
    std::map<std::string, RouteConfig> exactRoutes_;
    std::vector<PatternRoute> patternRoutes_;

    // Error configurations
    std::optional<ErrorConfig> globalError_;
    std::map<std::string, ErrorConfig> urlErrors_;

    // Request tracking
    std::vector<std::string> requestHistory_;

    // Configuration
    std::string fixtureBasePath_;
    std::optional<std::string> defaultResponse_;
    std::optional<long> timeout_;
    std::optional<std::string> userAgent_;
};

} // namespace testing
} // namespace rickmorty
