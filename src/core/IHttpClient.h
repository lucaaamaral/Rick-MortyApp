#pragma once

/**
 * @file IHttpClient.h
 * @brief HTTP client interface for dependency injection and testing support.
 *
 * This file defines the IHttpClient interface that abstracts HTTP operations,
 * enabling dependency injection for unit testing and allowing different HTTP
 * implementations to be swapped in.
 */

#include <string>
#include <stdexcept>

namespace rickmorty {

/**
 * @class HttpException
 * @brief Exception class for HTTP-level errors.
 *
 * This exception is thrown when HTTP operations fail at the transport level,
 * such as network errors, timeouts, or HTTP error status codes.
 * It is distinct from ApiException which handles higher-level API errors
 * like JSON parsing failures.
 */
class HttpException : public std::runtime_error {
public:
    /**
     * @enum Type
     * @brief Categorizes the type of HTTP error that occurred.
     */
    enum class Type {
        NetworkError,    ///< General network failure (connection refused, DNS error, etc.)
        InvalidResponse, ///< Server returned an invalid or malformed response
        NotFound,        ///< HTTP 404 - Resource not found
        Timeout          ///< Request timed out
    };

    /**
     * @brief Constructs an HttpException with the specified type, message, and HTTP code.
     * @param type The category of HTTP error.
     * @param message A descriptive error message.
     * @param httpCode The HTTP status code (0 if not applicable, e.g., for network errors).
     */
    HttpException(Type type, const std::string& message, int httpCode = 0)
        : std::runtime_error(message)
        , type_(type)
        , httpCode_(httpCode)
    {}

    /**
     * @brief Returns the type of HTTP error.
     * @return The error type enumeration value.
     */
    Type type() const noexcept { return type_; }

    /**
     * @brief Returns the HTTP status code associated with the error.
     * @return The HTTP status code, or 0 if not applicable.
     */
    int httpCode() const noexcept { return httpCode_; }

private:
    Type type_;
    int httpCode_;
};

/**
 * @class IHttpClient
 * @brief Abstract interface for HTTP client operations.
 *
 * This interface enables dependency injection for HTTP operations, allowing
 * the ApiClient to be tested with mock HTTP responses without making actual
 * network requests.
 *
 * Example usage:
 * @code
 * class MockHttpClient : public IHttpClient {
 * public:
 *     std::string get(const std::string& url) override {
 *         return R"({"id": 1, "name": "Test"})";
 *     }
 * };
 * @endcode
 */
class IHttpClient {
public:
    /**
     * @brief Virtual destructor for proper cleanup of derived classes.
     */
    virtual ~IHttpClient() = default;

    /**
     * @brief Performs an HTTP GET request to the specified URL.
     * @param url The complete URL to request.
     * @return The response body as a string.
     * @throws HttpException on network errors, timeouts, or HTTP error status codes.
     */
    virtual std::string get(const std::string& url) = 0;

    /**
     * @brief Sets the timeout for HTTP requests.
     * @param timeoutMs Timeout in milliseconds.
     *
     * This method has a default empty implementation to make it optional
     * for implementations that don't support configurable timeouts.
     */
    virtual void setTimeout(long timeoutMs) {
        (void)timeoutMs; // Suppress unused parameter warning
    }

    /**
     * @brief Sets the User-Agent header for HTTP requests.
     * @param userAgent The User-Agent string to use.
     *
     * This method has a default empty implementation to make it optional
     * for implementations that don't need custom User-Agent strings.
     */
    virtual void setUserAgent(const std::string& userAgent) {
        (void)userAgent; // Suppress unused parameter warning
    }
};

} // namespace rickmorty
