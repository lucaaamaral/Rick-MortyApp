#include "QmlBridge.h"
#include <QThreadPool>
#include <QMetaObject>
#include <glog/logging.h>

QmlBridge::QmlBridge(rickmorty::DataStore* dataStore, QObject* parent)
    : QObject(parent)
    , dataStore_(dataStore)
    , episodeModel_(this)
    , characterModel_(this)
{
    LOG(INFO) << "QmlBridge created, registering as observer";
    dataStore_->addObserver(this);
}

QmlBridge::~QmlBridge() {
    dataStore_->removeObserver(this);
}

void QmlBridge::loadEpisodes() {
    LOG(INFO) << "QmlBridge::loadEpisodes called";
    QThreadPool::globalInstance()->start([this]() {
        dataStore_->loadAllEpisodes();
    });
}

void QmlBridge::loadCharactersForEpisode(int episodeId) {
    LOG(INFO) << "[TRACE] QmlBridge::loadCharactersForEpisode called for episode " << episodeId
              << " (previous selectedEpisodeId_: " << selectedEpisodeId_ << ")";
    selectedEpisodeId_ = episodeId;

    auto episode = dataStore_->getEpisode(episodeId);
    if (episode) {
        selectedEpisodeName_ = QString::fromStdString(
            episode->episodeCode + " - " + episode->name
        );
        LOG(INFO) << "[TRACE] Set selectedEpisodeName_ to: " << selectedEpisodeName_.toStdString();
        emit selectedEpisodeChanged();
    }

    LOG(INFO) << "[TRACE] Queueing thread pool task for episode " << episodeId;
    QThreadPool::globalInstance()->start([this, episodeId]() {
        LOG(INFO) << "[TRACE] Thread pool task STARTING for episode " << episodeId;
        dataStore_->loadCharactersForEpisode(episodeId);
        LOG(INFO) << "[TRACE] Thread pool task FINISHED for episode " << episodeId;
    });
}

void QmlBridge::onEpisodesLoaded(const std::vector<rickmorty::Episode>& episodes) {
    LOG(INFO) << "QmlBridge::onEpisodesLoaded received " << episodes.size() << " episodes";
    QMetaObject::invokeMethod(this, [this, episodes]() {
        LOG(INFO) << "Setting episodes on model (UI thread)";
        episodeModel_.setEpisodes(episodes);
        LOG(INFO) << "Episode model now has " << episodeModel_.rowCount() << " rows";
        emit episodesReady();
    }, Qt::QueuedConnection);
}

void QmlBridge::onCharactersLoaded(int episodeId, const std::vector<rickmorty::Character>& characters) {
    LOG(INFO) << "[TRACE] onCharactersLoaded CALLED on thread, episodeId=" << episodeId
              << ", characters.size()=" << characters.size()
              << ", selectedEpisodeId_=" << selectedEpisodeId_;
    QMetaObject::invokeMethod(this, [this, episodeId, characters]() {
        LOG(INFO) << "[TRACE] onCharactersLoaded UI CALLBACK, episodeId=" << episodeId
                  << ", selectedEpisodeId_=" << selectedEpisodeId_;
        // Ignore callbacks for episodes that are no longer selected
        if (episodeId != selectedEpisodeId_) {
            LOG(INFO) << "[TRACE] IGNORING stale callback for episode " << episodeId
                      << " (current: " << selectedEpisodeId_ << ")";
            return;
        }
        LOG(INFO) << "[TRACE] Setting characters on model (UI thread) for episode " << episodeId;
        characterModel_.setCharacters(characters);
        LOG(INFO) << "[TRACE] Character model now has " << characterModel_.rowCount() << " rows";
        emit charactersReady(episodeId);
        LOG(INFO) << "[TRACE] Updating random character";
        updateRandomCharacter();
        LOG(INFO) << "[TRACE] onCharactersLoaded UI CALLBACK COMPLETE for episode " << episodeId;
    }, Qt::QueuedConnection);
}

void QmlBridge::onLoadingStateChanged(bool isLoading) {
    LOG(INFO) << "QmlBridge::onLoadingStateChanged: " << (isLoading ? "true" : "false");
    QMetaObject::invokeMethod(this, [this, isLoading]() {
        if (isLoading_ != isLoading) {
            isLoading_ = isLoading;
            emit loadingChanged();
        }
    }, Qt::QueuedConnection);
}

void QmlBridge::onError(const std::string& message) {
    LOG(ERROR) << "QmlBridge::onError: " << message;
    QMetaObject::invokeMethod(this, [this, message]() {
        errorMessage_ = QString::fromStdString(message);
        emit errorOccurred(errorMessage_);
    }, Qt::QueuedConnection);
}

int QmlBridge::cachedCharacterCount() const {
    return static_cast<int>(dataStore_->getCachedCharacterCount());
}

void QmlBridge::shuffleRandomCharacter() {
    updateRandomCharacter();
}

void QmlBridge::updateRandomCharacter() {
    auto character = dataStore_->getRandomCachedCharacter();
    if (character) {
        QVariantMap map;
        map["id"] = character->id;
        map["name"] = QString::fromStdString(character->name);
        map["status"] = QString::fromStdString(rickmorty::statusToString(character->status));
        map["species"] = QString::fromStdString(character->species);
        map["type"] = QString::fromStdString(character->type);
        map["gender"] = QString::fromStdString(rickmorty::genderToString(character->gender));
        map["originName"] = QString::fromStdString(character->origin.name);
        map["locationName"] = QString::fromStdString(character->location.name);
        map["imageUrl"] = QString::fromStdString(character->imageUrl);
        map["episodeCount"] = static_cast<int>(character->episodeIds.size());
        randomCharacter_ = map;
        emit randomCharacterChanged();
        emit cachedCharacterCountChanged();
    }
}
