#include "CharacterModel.h"

CharacterModel::CharacterModel(QObject* parent)
    : QAbstractListModel(parent) {}

int CharacterModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return characters_.size();
}

QVariant CharacterModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= characters_.size()) {
        return QVariant();
    }

    const auto& character = characters_.at(index.row());

    switch (role) {
        case IdRole:
            return character.id;
        case NameRole:
            return QString::fromStdString(character.name);
        case StatusRole:
            return QString::fromStdString(rickmorty::statusToString(character.status));
        case SpeciesRole:
            return QString::fromStdString(character.species);
        case TypeRole:
            return QString::fromStdString(character.type);
        case GenderRole:
            return QString::fromStdString(rickmorty::genderToString(character.gender));
        case OriginNameRole:
            return QString::fromStdString(character.origin.name);
        case OriginIdRole:
            return character.origin.id;
        case LocationNameRole:
            return QString::fromStdString(character.location.name);
        case LocationIdRole:
            return character.location.id;
        case ImageUrlRole:
            return QString::fromStdString(character.imageUrl);
        case EpisodeCountRole:
            return static_cast<int>(character.episodeIds.size());
        case UrlRole:
            return QString::fromStdString(character.url);
        case CreatedRole:
            return QString::fromStdString(character.created);
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> CharacterModel::roleNames() const {
    return {
        { IdRole, "id" },
        { NameRole, "name" },
        { StatusRole, "status" },
        { SpeciesRole, "species" },
        { TypeRole, "type" },
        { GenderRole, "gender" },
        { OriginNameRole, "originName" },
        { OriginIdRole, "originId" },
        { LocationNameRole, "locationName" },
        { LocationIdRole, "locationId" },
        { ImageUrlRole, "imageUrl" },
        { EpisodeCountRole, "episodeCount" },
        { UrlRole, "url" },
        { CreatedRole, "created" }
    };
}

void CharacterModel::setCharacters(const std::vector<rickmorty::Character>& characters) {
    beginResetModel();
    characters_.clear();
    for (const auto& ch : characters) {
        characters_.append(ch);
    }
    endResetModel();
    emit countChanged();
}

void CharacterModel::clear() {
    beginResetModel();
    characters_.clear();
    endResetModel();
    emit countChanged();
}
