#pragma once

/**
 * @file CurlHttpClient.h
 * @brief CURL-based implementation of the IHttpClient interface.
 *
 * This file provides a concrete HTTP client implementation using libcurl.
 * It handles HTTP GET requests with proper error handling and resource management.
 */

#include "IHttpClient.h"
#include <curl/curl.h>
#include <string>

namespace rickmorty {

/**
 * @class CurlHttpClient
 * @brief HTTP client implementation using libcurl.
 *
 * This class provides HTTP functionality using the libcurl library.
 * It manages the CURL handle lifecycle and translates CURL errors
 * to HttpException for consistent error handling.
 *
 * @note This class is NOT thread-safe. Each thread should have its own
 *       CurlHttpClient instance. The CURL handle is not designed to be
 *       shared across threads without external synchronization.
 *
 * @note This class manages curl_global_init/curl_global_cleanup. In applications
 *       with multiple CurlHttpClient instances, consider managing global CURL
 *       initialization separately to avoid redundant init/cleanup calls.
 *
 * Example usage:
 * @code
 * CurlHttpClient client;
 * client.setTimeout(5000);  // 5 second timeout
 * try {
 *     std::string response = client.get("https://api.example.com/data");
 * } catch (const HttpException& e) {
 *     // Handle error
 * }
 * @endcode
 */
class CurlHttpClient : public IHttpClient {
public:
    /**
     * @brief Constructs a CurlHttpClient and initializes the CURL handle.
     * @throws HttpException if CURL initialization fails.
     *
     * Initializes libcurl globally and creates a CURL easy handle.
     * Sets default options including follow redirects, 30-second timeout,
     * and a default User-Agent string.
     */
    CurlHttpClient();

    /**
     * @brief Destructor that cleans up CURL resources.
     *
     * Releases the CURL easy handle and performs global CURL cleanup.
     */
    ~CurlHttpClient() override;

    // Non-copyable to prevent multiple handles pointing to same resource
    CurlHttpClient(const CurlHttpClient&) = delete;
    CurlHttpClient& operator=(const CurlHttpClient&) = delete;

    // Movable for flexibility
    CurlHttpClient(CurlHttpClient&& other) noexcept;
    CurlHttpClient& operator=(CurlHttpClient&& other) noexcept;

    /**
     * @brief Performs an HTTP GET request to the specified URL.
     * @param url The complete URL to request.
     * @return The response body as a string.
     * @throws HttpException on network errors, timeouts, or HTTP error status codes.
     *
     * Error handling:
     * - CURL errors (connection failed, etc.) throw HttpException::Type::NetworkError
     * - HTTP 404 throws HttpException::Type::NotFound
     * - Other HTTP errors (4xx, 5xx) throw HttpException::Type::InvalidResponse
     * - Timeout errors throw HttpException::Type::Timeout
     */
    std::string get(const std::string& url) override;

    /**
     * @brief Sets the timeout for HTTP requests.
     * @param timeoutMs Timeout in milliseconds.
     *
     * The timeout applies to the entire request operation including
     * connection, transfer, etc. Default is 30000ms (30 seconds).
     */
    void setTimeout(long timeoutMs) override;

    /**
     * @brief Sets the User-Agent header for HTTP requests.
     * @param userAgent The User-Agent string to use.
     *
     * Default User-Agent is "RickAndMortyViewer/1.0".
     */
    void setUserAgent(const std::string& userAgent) override;

private:
    CURL* curl_;           ///< The CURL easy handle
    long timeoutMs_;       ///< Current timeout in milliseconds
    std::string userAgent_; ///< Current User-Agent string

    /**
     * @brief CURL write callback function for receiving response data.
     * @param ptr Pointer to the received data.
     * @param size Size of each data element.
     * @param nmemb Number of data elements.
     * @param data Pointer to the string to append data to.
     * @return Number of bytes processed.
     */
    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data);
};

} // namespace rickmorty
