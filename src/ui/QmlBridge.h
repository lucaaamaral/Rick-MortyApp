#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <memory>
#include "core/DataStore.h"
#include "core/Observer.h"
#include "EpisodeModel.h"
#include "CharacterModel.h"

class QmlBridge : public QObject, public rickmorty::IDataObserver {
    Q_OBJECT
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorOccurred)
    Q_PROPERTY(QString selectedEpisodeName READ selectedEpisodeName NOTIFY selectedEpisodeChanged)
    Q_PROPERTY(EpisodeModel* episodeModel READ episodeModel CONSTANT)
    Q_PROPERTY(CharacterModel* characterModel READ characterModel CONSTANT)
    Q_PROPERTY(int cachedCharacterCount READ cachedCharacterCount NOTIFY cachedCharacterCountChanged)
    Q_PROPERTY(QVariantMap randomCharacter READ randomCharacter NOTIFY randomCharacterChanged)

public:
    explicit QmlBridge(rickmorty::DataStore* dataStore, QObject* parent = nullptr);
    ~QmlBridge() override;

    bool isLoading() const { return isLoading_; }
    QString errorMessage() const { return errorMessage_; }
    QString selectedEpisodeName() const { return selectedEpisodeName_; }

    EpisodeModel* episodeModel() { return &episodeModel_; }
    CharacterModel* characterModel() { return &characterModel_; }
    int cachedCharacterCount() const;
    QVariantMap randomCharacter() const { return randomCharacter_; }

    Q_INVOKABLE void loadEpisodes();
    Q_INVOKABLE void loadCharactersForEpisode(int episodeId);
    Q_INVOKABLE void shuffleRandomCharacter();

    // IDataObserver implementation
    void onEpisodesLoaded(const std::vector<rickmorty::Episode>& episodes) override;
    void onCharactersLoaded(int episodeId, const std::vector<rickmorty::Character>& characters) override;
    void onLoadingStateChanged(bool isLoading) override;
    void onError(const std::string& message) override;

signals:
    void episodesReady();
    void charactersReady(int episodeId);
    void loadingChanged();
    void errorOccurred(const QString& message);
    void selectedEpisodeChanged();
    void cachedCharacterCountChanged();
    void randomCharacterChanged();

private:
    void updateRandomCharacter();

    rickmorty::DataStore* dataStore_;
    EpisodeModel episodeModel_;
    CharacterModel characterModel_;

    bool isLoading_ = false;
    QString errorMessage_;
    QString selectedEpisodeName_;
    int selectedEpisodeId_ = -1;
    QVariantMap randomCharacter_;
};
