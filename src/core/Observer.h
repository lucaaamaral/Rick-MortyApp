#pragma once

#include <vector>
#include <string>
#include "Models.h"

namespace rickmorty {

class IDataObserver {
public:
    virtual ~IDataObserver() = default;

    virtual void onEpisodesLoaded(const std::vector<Episode>& episodes) = 0;
    virtual void onCharactersLoaded(int episodeId, const std::vector<Character>& characters) = 0;
    virtual void onLoadingStateChanged(bool isLoading) = 0;
    virtual void onError(const std::string& message) = 0;
};

class IDataSubject {
public:
    virtual ~IDataSubject() = default;

    virtual void addObserver(IDataObserver* observer) = 0;
    virtual void removeObserver(IDataObserver* observer) = 0;

protected:
    virtual void notifyEpisodesLoaded(const std::vector<Episode>& episodes) = 0;
    virtual void notifyCharactersLoaded(int episodeId, const std::vector<Character>& characters) = 0;
    virtual void notifyLoadingStateChanged(bool isLoading) = 0;
    virtual void notifyError(const std::string& message) = 0;
};

} // namespace rickmorty
