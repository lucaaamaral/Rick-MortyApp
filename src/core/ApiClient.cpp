#include "ApiClient.h"
#include <sstream>
#include <glog/logging.h>

namespace rickmorty {

static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

ApiClient::ApiClient() {
    LOG(INFO) << "Initializing ApiClient";
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
    if (!curl_) {
        LOG(ERROR) << "Failed to initialize CURL";
        throw ApiException(ApiException::Type::NetworkError, "Failed to initialize CURL");
    }

    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, "RickAndMortyViewer/1.0");
    LOG(INFO) << "ApiClient initialized successfully";
}

ApiClient::~ApiClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
}

std::string ApiClient::httpGet(const std::string& url) {
    LOG(INFO) << "HTTP GET: " << url;
    std::string response;

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl_);
    if (res != CURLE_OK) {
        LOG(ERROR) << "CURL error: " << curl_easy_strerror(res);
        throw ApiException(ApiException::Type::NetworkError,
            "HTTP request failed: " + std::string(curl_easy_strerror(res)));
    }

    long httpCode = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &httpCode);
    LOG(INFO) << "HTTP response code: " << httpCode << ", size: " << response.size() << " bytes";

    if (httpCode == 404) {
        LOG(WARNING) << "Resource not found: " << url;
        throw ApiException(ApiException::Type::NotFound, "Resource not found");
    }

    if (httpCode < 200 || httpCode >= 300) {
        LOG(ERROR) << "HTTP error " << httpCode << " for URL: " << url;
        throw ApiException(ApiException::Type::NetworkError,
            "HTTP error: " + std::to_string(httpCode));
    }

    return response;
}

template<typename T>
std::vector<T> ApiClient::fetchAllPaginated(const std::string& endpoint) {
    LOG(INFO) << "Fetching all paginated: " << endpoint;
    std::vector<T> results;
    std::string url = std::string(BASE_URL) + endpoint;
    int page = 1;

    while (!url.empty()) {
        std::string response = httpGet(url);

        try {
            nlohmann::json j = nlohmann::json::parse(response);

            PaginationInfo info = j.at("info").get<PaginationInfo>();
            LOG(INFO) << "Page " << page << "/" << info.pages << " - total count: " << info.count;

            for (const auto& item : j.at("results")) {
                results.push_back(item.get<T>());
            }

            url = info.next.value_or("");
            page++;
        } catch (const nlohmann::json::exception& e) {
            LOG(ERROR) << "JSON parse error: " << e.what();
            LOG(ERROR) << "Response (first 500 chars): " << response.substr(0, 500);
            throw ApiException(ApiException::Type::ParseError,
                "JSON parse error: " + std::string(e.what()));
        }
    }

    LOG(INFO) << "Fetched " << results.size() << " items from " << endpoint;
    return results;
}

std::vector<Episode> ApiClient::fetchAllEpisodes() {
    return fetchAllPaginated<Episode>("/episode");
}

std::optional<Episode> ApiClient::fetchEpisode(int id) {
    try {
        std::string url = std::string(BASE_URL) + "/episode/" + std::to_string(id);
        std::string response = httpGet(url);

        nlohmann::json j = nlohmann::json::parse(response);
        return j.get<Episode>();
    } catch (const ApiException& e) {
        if (e.type() == ApiException::Type::NotFound) {
            return std::nullopt;
        }
        throw;
    } catch (const nlohmann::json::exception& e) {
        throw ApiException(ApiException::Type::ParseError,
            "JSON parse error: " + std::string(e.what()));
    }
}

std::vector<Character> ApiClient::fetchCharacters(const std::vector<int>& ids) {
    if (ids.empty()) {
        LOG(INFO) << "fetchCharacters called with empty ids";
        return {};
    }

    LOG(INFO) << "Fetching " << ids.size() << " characters";

    std::ostringstream idList;
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i > 0) idList << ",";
        idList << ids[i];
    }

    std::string url = std::string(BASE_URL) + "/character/" + idList.str();
    std::string response = httpGet(url);

    try {
        nlohmann::json j = nlohmann::json::parse(response);

        std::vector<Character> characters;
        if (j.is_array()) {
            characters = j.get<std::vector<Character>>();
        } else {
            characters = { j.get<Character>() };
        }

        LOG(INFO) << "Successfully parsed " << characters.size() << " characters";
        return characters;
    } catch (const nlohmann::json::exception& e) {
        LOG(ERROR) << "JSON parse error in fetchCharacters: " << e.what();
        LOG(ERROR) << "Response (first 500 chars): " << response.substr(0, 500);
        throw ApiException(ApiException::Type::ParseError,
            "JSON parse error: " + std::string(e.what()));
    }
}

std::optional<Character> ApiClient::fetchCharacter(int id) {
    try {
        std::string url = std::string(BASE_URL) + "/character/" + std::to_string(id);
        std::string response = httpGet(url);

        nlohmann::json j = nlohmann::json::parse(response);
        return j.get<Character>();
    } catch (const ApiException& e) {
        if (e.type() == ApiException::Type::NotFound) {
            return std::nullopt;
        }
        throw;
    } catch (const nlohmann::json::exception& e) {
        throw ApiException(ApiException::Type::ParseError,
            "JSON parse error: " + std::string(e.what()));
    }
}

std::optional<Location> ApiClient::fetchLocation(int id) {
    try {
        std::string url = std::string(BASE_URL) + "/location/" + std::to_string(id);
        std::string response = httpGet(url);

        nlohmann::json j = nlohmann::json::parse(response);
        return j.get<Location>();
    } catch (const ApiException& e) {
        if (e.type() == ApiException::Type::NotFound) {
            return std::nullopt;
        }
        throw;
    } catch (const nlohmann::json::exception& e) {
        throw ApiException(ApiException::Type::ParseError,
            "JSON parse error: " + std::string(e.what()));
    }
}

} // namespace rickmorty
