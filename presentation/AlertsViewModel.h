#pragma once

#include "AlertFilterModel.h"
#include "TelemetryItems.h"
#include "ViewModelBase.h"

#include <QAbstractListModel>
#include <QHash>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QVariantMap>
#include <QVariantList>
#include <QVector>

class TelemetryServiceBase;

class AlertListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        LevelRole = Qt::UserRole + 1,
        MessageRole,
        DeviceRole,
        TimeRole,
        EvidenceRole,
        ActionRole,
        AcknowledgedRole,
        MutedRole,
        IdRole
    };

    explicit AlertListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    const AlertItem *itemAt(int row) const;
    void setItems(const QVector<AlertItem> &items);
    bool updateStatus(int row, bool acknowledged, bool muted);

private:
    QVector<AlertItem> m_items;
};

class AlertsViewModel : public ViewModelBase
{
    Q_OBJECT
    Q_PROPERTY(AlertListModel* alerts READ alerts CONSTANT)
    Q_PROPERTY(AlertFilterModel* filteredAlerts READ filteredAlerts CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(QVariantMap selectedAlert READ selectedAlert NOTIFY selectedAlertChanged)
    Q_PROPERTY(QVariantList evidencePoints READ evidencePoints NOTIFY evidencePointsChanged)

public:
    explicit AlertsViewModel(TelemetryServiceBase *service = nullptr,
                             const QUrl &apiBase = QUrl(),
                             bool useRest = false,
                             QObject *parent = nullptr);

    AlertListModel *alerts();
    AlertFilterModel *filteredAlerts();
    int selectedIndex() const;
    void setSelectedIndex(int index);
    QVariantMap selectedAlert() const;
    QVariantList evidencePoints() const;

    Q_INVOKABLE QString exportAlerts();
    Q_INVOKABLE void acknowledgeSelected();
    Q_INVOKABLE void muteSelected();

signals:
    void selectedIndexChanged();
    void selectedAlertChanged();
    void evidencePointsChanged();

private:
    QString alertKey(const AlertItem &item) const;
    void handleAlertsChanged(int topRow, int bottomRow);
    void refreshAlerts();
    void ensureSelectionValid();
    void updateEvidencePoints();

    AlertListModel m_alerts;
    AlertFilterModel m_filteredAlerts;
    int m_selectedIndex = 0;
    TelemetryServiceBase *m_service = nullptr;
    QVariantList m_evidencePoints;
    QHash<QString, QPair<bool, bool>> m_statusByKey;
    bool m_useRest = false;
    QUrl m_apiBase;
    QNetworkAccessManager m_network;
};
