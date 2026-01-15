#pragma once

#include <QAbstractListModel>
#include <QList>
#include "core/Models.h"

class EpisodeModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        EpisodeCodeRole,
        AirDateRole,
        CharacterCountRole,
        SeasonRole,
        EpisodeNumberRole,
        UrlRole,
        CreatedRole
    };

    explicit EpisodeModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void setEpisodes(const std::vector<rickmorty::Episode>& episodes);
    void clear();

signals:
    void countChanged();

private:
    QList<rickmorty::Episode> episodes_;
};
