#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <algorithm>
#include <random>
#include "Models.h"
#include "Observer.h"
#include "ApiClient.h"

namespace rickmorty {

class DataStore : public IDataSubject {
public:
    explicit DataStore(std::unique_ptr<ApiClient> apiClient);
    ~DataStore() override = default;

    void addObserver(IDataObserver* observer) override;
    void removeObserver(IDataObserver* observer) override;

    void loadAllEpisodes();
    void loadCharactersForEpisode(int episodeId);

    const std::vector<Episode>& getEpisodes() const;
    std::vector<Character> getCharactersForEpisode(int episodeId) const;
    std::optional<Character> getCharacter(int id) const;
    std::optional<Episode> getEpisode(int id) const;

    bool areEpisodesLoaded() const;
    bool areCharactersLoadedForEpisode(int episodeId) const;

    // Random character support for showcase
    std::vector<Character> getAllCachedCharacters() const;
    std::optional<Character> getRandomCachedCharacter() const;
    size_t getCachedCharacterCount() const;

protected:
    void notifyEpisodesLoaded(const std::vector<Episode>& episodes) override;
    void notifyCharactersLoaded(int episodeId, const std::vector<Character>& characters) override;
    void notifyLoadingStateChanged(bool isLoading) override;
    void notifyError(const std::string& message) override;

private:
    // Internal unlocked version - caller must hold dataMutex_
    std::vector<Character> getCharactersForEpisodeUnlocked(int episodeId) const;

    std::unique_ptr<ApiClient> apiClient_;
    std::vector<IDataObserver*> observers_;
    mutable std::mutex observersMutex_;

    std::vector<Episode> episodes_;
    std::unordered_map<int, Character> characterCache_;
    std::unordered_set<int> loadedEpisodeCharacters_;
    mutable std::mutex dataMutex_;

    bool episodesLoaded_ = false;
};

} // namespace rickmorty
