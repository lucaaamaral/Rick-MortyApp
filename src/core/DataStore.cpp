#include "DataStore.h"
#include <glog/logging.h>

namespace rickmorty {

DataStore::DataStore(std::unique_ptr<ApiClient> apiClient)
    : apiClient_(std::move(apiClient)) {}

void DataStore::addObserver(IDataObserver* observer) {
    std::lock_guard<std::mutex> lock(observersMutex_);
    observers_.push_back(observer);
}

void DataStore::removeObserver(IDataObserver* observer) {
    std::lock_guard<std::mutex> lock(observersMutex_);
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), observer),
        observers_.end()
    );
}

void DataStore::loadAllEpisodes() {
    LOG(INFO) << "loadAllEpisodes called";

    if (episodesLoaded_) {
        LOG(INFO) << "Episodes already loaded, notifying observers";
        notifyEpisodesLoaded(episodes_);
        return;
    }

    notifyLoadingStateChanged(true);

    try {
        LOG(INFO) << "Fetching all episodes from API...";
        auto episodes = apiClient_->fetchAllEpisodes();
        LOG(INFO) << "Received " << episodes.size() << " episodes from API";

        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            episodes_ = std::move(episodes);
            episodesLoaded_ = true;
        }

        notifyLoadingStateChanged(false);
        notifyEpisodesLoaded(episodes_);
        LOG(INFO) << "Episodes loaded and observers notified";

    } catch (const std::exception& e) {
        LOG(ERROR) << "Error loading episodes: " << e.what();
        notifyLoadingStateChanged(false);
        notifyError(e.what());
    }
}

void DataStore::loadCharactersForEpisode(int episodeId) {
    LOG(INFO) << "[TRACE] loadCharactersForEpisode START for episode " << episodeId;

    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        if (loadedEpisodeCharacters_.count(episodeId)) {
            LOG(INFO) << "[TRACE] Episode " << episodeId << " already in cache, returning cached data";
            auto characters = getCharactersForEpisodeUnlocked(episodeId);  // Use unlocked version
            LOG(INFO) << "[TRACE] Notifying with " << characters.size() << " cached characters for episode " << episodeId;
            notifyCharactersLoaded(episodeId, characters);
            return;
        }
    }

    notifyLoadingStateChanged(true);

    try {
        // Copy character IDs while holding the lock to avoid stale pointer issues
        std::vector<int> characterIds;
        std::string episodeName;
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            auto it = std::find_if(episodes_.begin(), episodes_.end(),
                [episodeId](const Episode& e) { return e.id == episodeId; });

            if (it == episodes_.end()) {
                LOG(ERROR) << "[TRACE] Episode " << episodeId << " not found in cache";
                throw std::runtime_error("Episode not found: " + std::to_string(episodeId));
            }
            // Copy data while holding lock - don't keep pointer
            characterIds = it->characterIds;
            episodeName = it->name;
            LOG(INFO) << "[TRACE] Found episode: " << episodeName << " with " << characterIds.size() << " characters";
        }

        std::vector<int> toFetch;
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            for (int charId : characterIds) {
                if (characterCache_.find(charId) == characterCache_.end()) {
                    toFetch.push_back(charId);
                }
            }
        }

        LOG(INFO) << "[TRACE] Need to fetch " << toFetch.size() << " characters (out of " << characterIds.size() << ") for episode " << episodeId;

        if (!toFetch.empty()) {
            auto fetched = apiClient_->fetchCharacters(toFetch);
            LOG(INFO) << "Fetched " << fetched.size() << " characters from API";

            std::lock_guard<std::mutex> lock(dataMutex_);
            for (auto& c : fetched) {
                characterCache_[c.id] = std::move(c);
            }
        }

        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            loadedEpisodeCharacters_.insert(episodeId);
        }

        LOG(INFO) << "[TRACE] Getting characters for episode " << episodeId;
        auto characters = getCharactersForEpisode(episodeId);
        LOG(INFO) << "[TRACE] Final character list has " << characters.size() << " characters for episode " << episodeId;

        LOG(INFO) << "[TRACE] Notifying loading state false for episode " << episodeId;
        notifyLoadingStateChanged(false);
        LOG(INFO) << "[TRACE] Notifying characters loaded for episode " << episodeId;
        notifyCharactersLoaded(episodeId, characters);
        LOG(INFO) << "[TRACE] loadCharactersForEpisode COMPLETE for episode " << episodeId;

    } catch (const std::exception& e) {
        LOG(ERROR) << "[TRACE] ERROR loading characters for episode " << episodeId << ": " << e.what();
        notifyLoadingStateChanged(false);
        notifyError(e.what());
    }
}

const std::vector<Episode>& DataStore::getEpisodes() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return episodes_;
}

// Internal version - assumes lock is already held
std::vector<Character> DataStore::getCharactersForEpisodeUnlocked(int episodeId) const {
    std::vector<Character> result;

    auto it = std::find_if(episodes_.begin(), episodes_.end(),
        [episodeId](const Episode& e) { return e.id == episodeId; });

    if (it != episodes_.end()) {
        for (int charId : it->characterIds) {
            auto charIt = characterCache_.find(charId);
            if (charIt != characterCache_.end()) {
                result.push_back(charIt->second);
            }
        }
    }

    std::sort(result.begin(), result.end());

    return result;
}

std::vector<Character> DataStore::getCharactersForEpisode(int episodeId) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return getCharactersForEpisodeUnlocked(episodeId);
}

std::optional<Character> DataStore::getCharacter(int id) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    auto it = characterCache_.find(id);
    if (it != characterCache_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<Episode> DataStore::getEpisode(int id) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    auto it = std::find_if(episodes_.begin(), episodes_.end(),
        [id](const Episode& e) { return e.id == id; });

    if (it != episodes_.end()) {
        return *it;
    }
    return std::nullopt;
}

bool DataStore::areEpisodesLoaded() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return episodesLoaded_;
}

bool DataStore::areCharactersLoadedForEpisode(int episodeId) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return loadedEpisodeCharacters_.count(episodeId) > 0;
}

std::vector<Character> DataStore::getAllCachedCharacters() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    std::vector<Character> result;
    result.reserve(characterCache_.size());
    for (const auto& [id, character] : characterCache_) {
        result.push_back(character);
    }
    return result;
}

std::optional<Character> DataStore::getRandomCachedCharacter() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    if (characterCache_.empty()) {
        return std::nullopt;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<size_t> dist(0, characterCache_.size() - 1);
    auto it = characterCache_.begin();
    std::advance(it, dist(gen));
    return it->second;
}

size_t DataStore::getCachedCharacterCount() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return characterCache_.size();
}

void DataStore::notifyEpisodesLoaded(const std::vector<Episode>& episodes) {
    std::lock_guard<std::mutex> lock(observersMutex_);
    for (auto* observer : observers_) {
        observer->onEpisodesLoaded(episodes);
    }
}

void DataStore::notifyCharactersLoaded(int episodeId, const std::vector<Character>& characters) {
    std::lock_guard<std::mutex> lock(observersMutex_);
    for (auto* observer : observers_) {
        observer->onCharactersLoaded(episodeId, characters);
    }
}

void DataStore::notifyLoadingStateChanged(bool isLoading) {
    std::lock_guard<std::mutex> lock(observersMutex_);
    for (auto* observer : observers_) {
        observer->onLoadingStateChanged(isLoading);
    }
}

void DataStore::notifyError(const std::string& message) {
    std::lock_guard<std::mutex> lock(observersMutex_);
    for (auto* observer : observers_) {
        observer->onError(message);
    }
}

} // namespace rickmorty
