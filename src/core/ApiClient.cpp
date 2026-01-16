#include "ApiClient.h"
#include "CurlHttpClient.h"
#include <sstream>
#include <glog/logging.h>

namespace rickmorty {

ApiClient::ApiClient()
    : httpClient_(std::make_unique<CurlHttpClient>())
{
    LOG(INFO) << "Initializing ApiClient with default CurlHttpClient";
}

ApiClient::ApiClient(std::unique_ptr<IHttpClient> httpClient)
    : httpClient_(std::move(httpClient))
{
    LOG(INFO) << "Initializing ApiClient with injected HTTP client";
    if (!httpClient_) {
        throw ApiException(ApiException::Type::NetworkError, "HTTP client cannot be null");
    }
}

ApiClient::~ApiClient() = default;

template<typename T>
std::vector<T> ApiClient::fetchAllPaginated(const std::string& endpoint) {
    LOG(INFO) << "Fetching all paginated: " << endpoint;
    std::vector<T> results;
    std::string url = std::string(BASE_URL) + endpoint;
    int page = 1;

    while (!url.empty()) {
        std::string response;
        try {
            response = httpClient_->get(url);
        } catch (const HttpException& e) {
            // Convert HttpException to ApiException
            switch (e.type()) {
                case HttpException::Type::NotFound:
                    throw ApiException(ApiException::Type::NotFound, e.what());
                case HttpException::Type::Timeout:
                case HttpException::Type::NetworkError:
                    throw ApiException(ApiException::Type::NetworkError, e.what());
                case HttpException::Type::InvalidResponse:
                default:
                    throw ApiException(ApiException::Type::NetworkError, e.what());
            }
        }

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
        std::string response = httpClient_->get(url);

        nlohmann::json j = nlohmann::json::parse(response);
        return j.get<Episode>();
    } catch (const HttpException& e) {
        if (e.type() == HttpException::Type::NotFound) {
            return std::nullopt;
        }
        // Convert other HttpException types to ApiException
        throw ApiException(ApiException::Type::NetworkError, e.what());
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
    std::string response;
    try {
        response = httpClient_->get(url);
    } catch (const HttpException& e) {
        // Convert HttpException to ApiException
        switch (e.type()) {
            case HttpException::Type::NotFound:
                throw ApiException(ApiException::Type::NotFound, e.what());
            case HttpException::Type::Timeout:
            case HttpException::Type::NetworkError:
            case HttpException::Type::InvalidResponse:
            default:
                throw ApiException(ApiException::Type::NetworkError, e.what());
        }
    }

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
        std::string response = httpClient_->get(url);

        nlohmann::json j = nlohmann::json::parse(response);
        return j.get<Character>();
    } catch (const HttpException& e) {
        if (e.type() == HttpException::Type::NotFound) {
            return std::nullopt;
        }
        // Convert other HttpException types to ApiException
        throw ApiException(ApiException::Type::NetworkError, e.what());
    } catch (const nlohmann::json::exception& e) {
        throw ApiException(ApiException::Type::ParseError,
            "JSON parse error: " + std::string(e.what()));
    }
}

std::optional<Location> ApiClient::fetchLocation(int id) {
    try {
        std::string url = std::string(BASE_URL) + "/location/" + std::to_string(id);
        std::string response = httpClient_->get(url);

        nlohmann::json j = nlohmann::json::parse(response);
        return j.get<Location>();
    } catch (const HttpException& e) {
        if (e.type() == HttpException::Type::NotFound) {
            return std::nullopt;
        }
        // Convert other HttpException types to ApiException
        throw ApiException(ApiException::Type::NetworkError, e.what());
    } catch (const nlohmann::json::exception& e) {
        throw ApiException(ApiException::Type::ParseError,
            "JSON parse error: " + std::string(e.what()));
    }
}

} // namespace rickmorty
