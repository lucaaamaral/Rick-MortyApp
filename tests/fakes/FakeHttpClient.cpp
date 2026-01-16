#include "FakeHttpClient.h"
#include <algorithm>
#include <stdexcept>

namespace rickmorty {
namespace testing {

//=============================================================================
// IHttpClient interface implementation
//=============================================================================

std::string FakeHttpClient::get(const std::string& url) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Record the request
    requestHistory_.push_back(url);

    // Check for global error simulation
    if (globalError_) {
        throw HttpException(globalError_->type, globalError_->message, globalError_->httpCode);
    }

    // Check for URL-specific error simulation
    auto errorIt = urlErrors_.find(url);
    if (errorIt != urlErrors_.end()) {
        throw HttpException(errorIt->second.type, errorIt->second.message, errorIt->second.httpCode);
    }

    // Check for exact URL match
    auto exactIt = exactRoutes_.find(url);
    if (exactIt != exactRoutes_.end()) {
        return getRouteResponse(exactIt->second, url);
    }

    // Check pattern routes in order
    for (const auto& patternRoute : patternRoutes_) {
        if (std::regex_search(url, patternRoute.pattern)) {
            return getRouteResponse(patternRoute.config, url);
        }
    }

    // Check for default response
    if (defaultResponse_) {
        return *defaultResponse_;
    }

    // No matching route found
    throw HttpException(
        HttpException::Type::NotFound,
        "FakeHttpClient: No route configured for URL: " + url,
        404);
}

void FakeHttpClient::setTimeout(long timeoutMs) {
    std::lock_guard<std::mutex> lock(mutex_);
    timeout_ = timeoutMs;
}

void FakeHttpClient::setUserAgent(const std::string& userAgent) {
    std::lock_guard<std::mutex> lock(mutex_);
    userAgent_ = userAgent;
}

//=============================================================================
// Fluent configuration API
//=============================================================================

FakeHttpClient& FakeHttpClient::route(const std::string& url, const std::string& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    RouteConfig config;
    config.type = RouteConfig::Type::StaticResponse;
    config.response = response;
    exactRoutes_[url] = config;
    return *this;
}

FakeHttpClient& FakeHttpClient::routeToFixture(const std::string& url, const std::string& fixturePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    RouteConfig config;
    config.type = RouteConfig::Type::FixtureFile;
    config.fixturePath = fixturePath;
    exactRoutes_[url] = config;
    return *this;
}

FakeHttpClient& FakeHttpClient::routePattern(const std::string& pattern, const std::string& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    PatternRoute patternRoute;
    patternRoute.pattern = std::regex(pattern);
    patternRoute.patternString = pattern;
    patternRoute.config.type = RouteConfig::Type::StaticResponse;
    patternRoute.config.response = response;
    patternRoutes_.push_back(std::move(patternRoute));
    return *this;
}

FakeHttpClient& FakeHttpClient::routePatternToFixture(const std::string& pattern, const std::string& fixturePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    PatternRoute patternRoute;
    patternRoute.pattern = std::regex(pattern);
    patternRoute.patternString = pattern;
    patternRoute.config.type = RouteConfig::Type::FixtureFile;
    patternRoute.config.fixturePath = fixturePath;
    patternRoutes_.push_back(std::move(patternRoute));
    return *this;
}

FakeHttpClient& FakeHttpClient::routePatternWithHandler(
    const std::string& pattern,
    std::function<std::string(const std::string&)> handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    PatternRoute patternRoute;
    patternRoute.pattern = std::regex(pattern);
    patternRoute.patternString = pattern;
    patternRoute.config.type = RouteConfig::Type::Handler;
    patternRoute.config.handler = std::move(handler);
    patternRoutes_.push_back(std::move(patternRoute));
    return *this;
}

FakeHttpClient& FakeHttpClient::simulateError(
    HttpException::Type type,
    const std::string& message,
    int httpCode) {
    std::lock_guard<std::mutex> lock(mutex_);
    globalError_ = ErrorConfig{type, message, httpCode};
    return *this;
}

FakeHttpClient& FakeHttpClient::simulateErrorForUrl(
    const std::string& url,
    HttpException::Type type,
    const std::string& message,
    int httpCode) {
    std::lock_guard<std::mutex> lock(mutex_);
    urlErrors_[url] = ErrorConfig{type, message, httpCode};
    return *this;
}

FakeHttpClient& FakeHttpClient::clearGlobalError() {
    std::lock_guard<std::mutex> lock(mutex_);
    globalError_.reset();
    return *this;
}

FakeHttpClient& FakeHttpClient::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    exactRoutes_.clear();
    patternRoutes_.clear();
    globalError_.reset();
    urlErrors_.clear();
    defaultResponse_.reset();
    // Note: We don't clear timeout_, userAgent_, fixtureBasePath_, or requestHistory_
    // as those are typically set once and used throughout a test.
    return *this;
}

FakeHttpClient& FakeHttpClient::setFixtureBasePath(const std::string& basePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    fixtureBasePath_ = basePath;
    // Ensure the path ends with a separator
    if (!fixtureBasePath_.empty() && fixtureBasePath_.back() != '/') {
        fixtureBasePath_ += '/';
    }
    return *this;
}

FakeHttpClient& FakeHttpClient::setDefaultResponse(const std::string& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    defaultResponse_ = response;
    return *this;
}

FakeHttpClient& FakeHttpClient::clearDefaultResponse() {
    std::lock_guard<std::mutex> lock(mutex_);
    defaultResponse_.reset();
    return *this;
}

//=============================================================================
// Verification API
//=============================================================================

std::vector<std::string> FakeHttpClient::requestedUrls() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return requestHistory_;
}

size_t FakeHttpClient::requestCount(const std::string& url) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::count(requestHistory_.begin(), requestHistory_.end(), url);
}

size_t FakeHttpClient::totalRequestCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return requestHistory_.size();
}

bool FakeHttpClient::wasRequested(const std::string& url) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::find(requestHistory_.begin(), requestHistory_.end(), url) != requestHistory_.end();
}

bool FakeHttpClient::wasRequestedMatching(const std::string& pattern) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::regex re(pattern);
    return std::any_of(requestHistory_.begin(), requestHistory_.end(),
        [&re](const std::string& url) {
            return std::regex_search(url, re);
        });
}

void FakeHttpClient::clearRequestHistory() {
    std::lock_guard<std::mutex> lock(mutex_);
    requestHistory_.clear();
}

std::optional<long> FakeHttpClient::configuredTimeout() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return timeout_;
}

std::optional<std::string> FakeHttpClient::configuredUserAgent() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return userAgent_;
}

//=============================================================================
// Private helper methods
//=============================================================================

std::string FakeHttpClient::loadFixture(const std::string& path) const {
    // Build full path
    std::string fullPath = path;
    if (!fixtureBasePath_.empty() && path[0] != '/') {
        fullPath = fixtureBasePath_ + path;
    }

    std::ifstream file(fullPath);
    if (!file.is_open()) {
        throw HttpException(
            HttpException::Type::InvalidResponse,
            "FakeHttpClient: Failed to load fixture file: " + fullPath,
            500);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string FakeHttpClient::getRouteResponse(const RouteConfig& config, const std::string& url) const {
    switch (config.type) {
        case RouteConfig::Type::StaticResponse:
            return config.response;

        case RouteConfig::Type::FixtureFile:
            return loadFixture(config.fixturePath);

        case RouteConfig::Type::Handler:
            if (config.handler) {
                return config.handler(url);
            }
            throw HttpException(
                HttpException::Type::InvalidResponse,
                "FakeHttpClient: Handler is null for URL: " + url,
                500);

        default:
            throw HttpException(
                HttpException::Type::InvalidResponse,
                "FakeHttpClient: Unknown route config type for URL: " + url,
                500);
    }
}

void FakeHttpClient::recordRequest(const std::string& url) {
    // Note: This method is not currently used since we record directly in get(),
    // but it's here for potential future use with async operations.
    std::lock_guard<std::mutex> lock(mutex_);
    requestHistory_.push_back(url);
}

} // namespace testing
} // namespace rickmorty
