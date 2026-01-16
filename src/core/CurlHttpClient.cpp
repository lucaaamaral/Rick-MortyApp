#include "CurlHttpClient.h"
#include <glog/logging.h>

namespace rickmorty {

size_t CurlHttpClient::writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

CurlHttpClient::CurlHttpClient()
    : curl_(nullptr)
    , timeoutMs_(30000)
    , userAgent_("RickAndMortyViewer/1.0")
{
    LOG(INFO) << "Initializing CurlHttpClient";
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
    if (!curl_) {
        LOG(ERROR) << "Failed to initialize CURL";
        throw HttpException(HttpException::Type::NetworkError, "Failed to initialize CURL");
    }

    // Set default options
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeoutMs_);
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, userAgent_.c_str());
    LOG(INFO) << "CurlHttpClient initialized successfully";
}

CurlHttpClient::~CurlHttpClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }
    curl_global_cleanup();
}

CurlHttpClient::CurlHttpClient(CurlHttpClient&& other) noexcept
    : curl_(other.curl_)
    , timeoutMs_(other.timeoutMs_)
    , userAgent_(std::move(other.userAgent_))
{
    other.curl_ = nullptr;
}

CurlHttpClient& CurlHttpClient::operator=(CurlHttpClient&& other) noexcept {
    if (this != &other) {
        if (curl_) {
            curl_easy_cleanup(curl_);
        }
        curl_ = other.curl_;
        timeoutMs_ = other.timeoutMs_;
        userAgent_ = std::move(other.userAgent_);
        other.curl_ = nullptr;
    }
    return *this;
}

std::string CurlHttpClient::get(const std::string& url) {
    LOG(INFO) << "HTTP GET: " << url;
    std::string response;

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl_);

    // Handle CURL-level errors
    if (res != CURLE_OK) {
        const char* errorMsg = curl_easy_strerror(res);
        LOG(ERROR) << "CURL error: " << errorMsg;

        // Map specific CURL errors to appropriate HttpException types
        if (res == CURLE_OPERATION_TIMEDOUT) {
            throw HttpException(HttpException::Type::Timeout,
                "HTTP request timed out: " + std::string(errorMsg));
        }
        throw HttpException(HttpException::Type::NetworkError,
            "HTTP request failed: " + std::string(errorMsg));
    }

    // Get HTTP response code
    long httpCode = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &httpCode);
    LOG(INFO) << "HTTP response code: " << httpCode << ", size: " << response.size() << " bytes";

    // Handle HTTP-level errors
    if (httpCode == 404) {
        LOG(WARNING) << "Resource not found: " << url;
        throw HttpException(HttpException::Type::NotFound, "Resource not found", 404);
    }

    if (httpCode < 200 || httpCode >= 300) {
        LOG(ERROR) << "HTTP error " << httpCode << " for URL: " << url;
        throw HttpException(HttpException::Type::InvalidResponse,
            "HTTP error: " + std::to_string(httpCode), static_cast<int>(httpCode));
    }

    return response;
}

void CurlHttpClient::setTimeout(long timeoutMs) {
    timeoutMs_ = timeoutMs;
    if (curl_) {
        curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeoutMs_);
    }
}

void CurlHttpClient::setUserAgent(const std::string& userAgent) {
    userAgent_ = userAgent;
    if (curl_) {
        curl_easy_setopt(curl_, CURLOPT_USERAGENT, userAgent_.c_str());
    }
}

} // namespace rickmorty
