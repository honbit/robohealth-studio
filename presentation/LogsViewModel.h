#pragma once

#include "LogFilterModel.h"
#include "TelemetryItems.h"
#include "ViewModelBase.h"

#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

class TelemetryServiceBase;

class LogListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        LevelRole = Qt::UserRole + 1,
        MessageRole,
        DeviceRole,
        TimeRole
    };

    explicit LogListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    const LogItem *itemAt(int row) const;
    void setItems(const QVector<LogItem> &items);

private:
    QVector<LogItem> m_items;
};

class LogsViewModel : public ViewModelBase
{
    Q_OBJECT
    Q_PROPERTY(LogListModel* logs READ logs CONSTANT)
    Q_PROPERTY(LogFilterModel* filteredLogs READ filteredLogs CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(QVariantMap selectedLog READ selectedLog NOTIFY selectedLogChanged)
    Q_PROPERTY(QVariantList contextLines READ contextLines NOTIFY contextLinesChanged)

public:
    explicit LogsViewModel(TelemetryServiceBase *service = nullptr,
                           const QUrl &apiBase = QUrl(),
                           bool useRest = false,
                           QObject *parent = nullptr);

    LogListModel *logs();
    LogFilterModel *filteredLogs();
    int selectedIndex() const;
    void setSelectedIndex(int index);
    QVariantMap selectedLog() const;
    QVariantList contextLines() const;

    Q_INVOKABLE QString exportLogs();

signals:
    void selectedIndexChanged();
    void selectedLogChanged();
    void contextLinesChanged();

private:
    void handleLogsChanged(int topRow, int bottomRow);
    void refreshLogs();
    void ensureSelectionValid();
    void updateContextLines();

    LogListModel m_logs;
    LogFilterModel m_filteredLogs;
    int m_selectedIndex = 0;
    TelemetryServiceBase *m_service = nullptr;
    QVariantList m_contextLines;
    bool m_useRest = false;
    QUrl m_apiBase;
    QNetworkAccessManager m_network;
};
