#include "EpisodeModel.h"

EpisodeModel::EpisodeModel(QObject* parent)
    : QAbstractListModel(parent) {}

int EpisodeModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return episodes_.size();
}

QVariant EpisodeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= episodes_.size()) {
        return QVariant();
    }

    const auto& episode = episodes_.at(index.row());

    switch (role) {
        case IdRole:
            return episode.id;
        case NameRole:
            return QString::fromStdString(episode.name);
        case EpisodeCodeRole:
            return QString::fromStdString(episode.episodeCode);
        case AirDateRole:
            return QString::fromStdString(episode.airDate);
        case CharacterCountRole:
            return static_cast<int>(episode.characterIds.size());
        case SeasonRole:
            return episode.season;
        case EpisodeNumberRole:
            return episode.episodeNumber;
        case UrlRole:
            return QString::fromStdString(episode.url);
        case CreatedRole:
            return QString::fromStdString(episode.created);
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> EpisodeModel::roleNames() const {
    return {
        { IdRole, "id" },
        { NameRole, "name" },
        { EpisodeCodeRole, "episodeCode" },
        { AirDateRole, "airDate" },
        { CharacterCountRole, "characterCount" },
        { SeasonRole, "season" },
        { EpisodeNumberRole, "episodeNumber" },
        { UrlRole, "url" },
        { CreatedRole, "created" }
    };
}

void EpisodeModel::setEpisodes(const std::vector<rickmorty::Episode>& episodes) {
    beginResetModel();
    episodes_.clear();
    for (const auto& ep : episodes) {
        episodes_.append(ep);
    }
    endResetModel();
    emit countChanged();
}

void EpisodeModel::clear() {
    beginResetModel();
    episodes_.clear();
    endResetModel();
    emit countChanged();
}
