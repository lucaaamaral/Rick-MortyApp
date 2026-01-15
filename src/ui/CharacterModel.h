#pragma once

#include <QAbstractListModel>
#include <QList>
#include "core/Models.h"

class CharacterModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        StatusRole,
        SpeciesRole,
        TypeRole,
        GenderRole,
        OriginNameRole,
        OriginIdRole,
        LocationNameRole,
        LocationIdRole,
        ImageUrlRole,
        EpisodeCountRole,
        UrlRole,
        CreatedRole
    };

    explicit CharacterModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void setCharacters(const std::vector<rickmorty::Character>& characters);
    void clear();

signals:
    void countChanged();

private:
    QList<rickmorty::Character> characters_;
};
