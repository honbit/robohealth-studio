#include "AlertsViewModel.h"

#include "RestExportHelper.h"
#include "TelemetryServiceBase.h"

#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QSet>
#include <QTextStream>
#include <QUrlQuery>

namespace {
QString csvEscape(const QString &value)
{
    QString out = value;
    out.replace("\"", "\"\"");
    if (out.contains(',') || out.contains('"') || out.contains('\n') || out.contains('\r')) {
        out = "\"" + out + "\"";
    }
    return out;
}

QString exportFilePath(const QString &baseName)
{
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (baseDir.isEmpty()) {
        baseDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }
    if (baseDir.isEmpty()) {
        baseDir = QDir::tempPath();
    }
    QDir dir(baseDir);
    const QString exportDirName = QStringLiteral("robohealth-exports");
    if (!dir.mkpath(exportDirName)) {
        return {};
    }
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    return dir.filePath(exportDirName + "/" + baseName + "_" + timestamp + ".csv");
}
}

AlertListModel::AlertListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_items = {
        {"alert-1", "Critical", "Joint torque spike", "Robot-02", "10:42", "Torque > 20N*m", "Stop & inspect"},
        {"alert-2", "Warn", "Vibration above threshold", "Robot-01", "10:37", "Vibration > 0.4g", "Check bearing"},
        {"alert-3", "Info", "Camera reconnect", "Cam-04", "10:20", "Link restored", "None"}
    };
}

int AlertListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.size();
}

QVariant AlertListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return {};
    }
    const AlertItem &item = m_items.at(index.row());
    switch (role) {
    case LevelRole: return item.level;
    case MessageRole: return item.message;
    case DeviceRole: return item.device;
    case TimeRole: return item.time;
    case EvidenceRole: return item.evidence;
    case ActionRole: return item.action;
    case AcknowledgedRole: return item.acknowledged;
    case MutedRole: return item.muted;
    case IdRole: return item.id;
    default: return {};
    }
}

QHash<int, QByteArray> AlertListModel::roleNames() const
{
    return {
        {LevelRole, "level"},
        {MessageRole, "message"},
        {DeviceRole, "device"},
        {TimeRole, "time"},
        {EvidenceRole, "evidence"},
        {ActionRole, "action"},
        {AcknowledgedRole, "acknowledged"},
        {MutedRole, "muted"},
        {IdRole, "id"}
    };
}

const AlertItem *AlertListModel::itemAt(int row) const
{
    if (row < 0 || row >= m_items.size()) {
        return nullptr;
    }
    return &m_items[row];
}

void AlertListModel::setItems(const QVector<AlertItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

bool AlertListModel::updateStatus(int row, bool acknowledged, bool muted)
{
    if (row < 0 || row >= m_items.size()) {
        return false;
    }
    AlertItem &item = m_items[row];
    if (item.acknowledged == acknowledged && item.muted == muted) {
        return false;
    }
    item.acknowledged = acknowledged;
    item.muted = muted;
    const QModelIndex idx = index(row, 0);
    emit dataChanged(idx, idx, {AcknowledgedRole, MutedRole});
    return true;
}

AlertsViewModel::AlertsViewModel(TelemetryServiceBase *service, const QUrl &apiBase, bool useRest, QObject *parent)
    : ViewModelBase(parent)
    , m_service(service)
    , m_useRest(useRest)
    , m_apiBase(apiBase)
{
    if (m_useRest && !m_apiBase.isValid()) {
        m_apiBase = QUrl(QStringLiteral("http://127.0.0.1:5000"));
    }
    m_filteredAlerts.setSourceModel(&m_alerts);

    connect(&m_filteredAlerts, &QAbstractItemModel::modelReset, this, &AlertsViewModel::ensureSelectionValid);
    connect(&m_filteredAlerts, &QAbstractItemModel::rowsInserted, this, &AlertsViewModel::ensureSelectionValid);
    connect(&m_filteredAlerts, &QAbstractItemModel::rowsRemoved, this, &AlertsViewModel::ensureSelectionValid);
    connect(&m_filteredAlerts, &QAbstractItemModel::layoutChanged, this, &AlertsViewModel::ensureSelectionValid);

    connect(&m_alerts, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
                handleAlertsChanged(topLeft.row(), bottomRight.row());
            });

    if (m_service) {
        refreshAlerts();
        connect(m_service, &TelemetryServiceBase::alertsUpdated, this, &AlertsViewModel::refreshAlerts);
    }

    updateEvidencePoints();
}

AlertListModel *AlertsViewModel::alerts()
{
    return &m_alerts;
}

AlertFilterModel *AlertsViewModel::filteredAlerts()
{
    return &m_filteredAlerts;
}

int AlertsViewModel::selectedIndex() const
{
    return m_selectedIndex;
}

void AlertsViewModel::setSelectedIndex(int index)
{
    if (m_selectedIndex == index) {
        return;
    }
    m_selectedIndex = index;
    emit selectedIndexChanged();
    emit selectedAlertChanged();
    updateEvidencePoints();
}

QVariantMap AlertsViewModel::selectedAlert() const
{
    QVariantMap map;
    const QModelIndex proxyIndex = m_filteredAlerts.index(m_selectedIndex, 0);
    if (!proxyIndex.isValid()) {
        return map;
    }
    const QModelIndex sourceIndex = m_filteredAlerts.mapToSource(proxyIndex);
    const AlertItem *item = m_alerts.itemAt(sourceIndex.row());
    if (!item) {
        return map;
    }
    map.insert("level", item->level);
    map.insert("message", item->message);
    map.insert("device", item->device);
    map.insert("time", item->time);
    map.insert("evidence", item->evidence);
    map.insert("action", item->action);
    map.insert("acknowledged", item->acknowledged);
    map.insert("muted", item->muted);
    map.insert("id", item->id);
    return map;
}

QVariantList AlertsViewModel::evidencePoints() const
{
    return m_evidencePoints;
}

QString AlertsViewModel::exportAlerts()
{
    if (m_useRest && m_apiBase.isValid()) {
        QUrl url = RestExport::resolveApiUrl(m_apiBase, QStringLiteral("/api/exports/alerts"));
        QUrlQuery query;
        if (!m_filteredAlerts.levelFilter().isEmpty()) {
            query.addQueryItem(QStringLiteral("level"), m_filteredAlerts.levelFilter());
        }
        if (!m_filteredAlerts.deviceFilter().isEmpty()) {
            query.addQueryItem(QStringLiteral("device"), m_filteredAlerts.deviceFilter());
        }
        if (!m_filteredAlerts.timeRange().isEmpty()) {
            query.addQueryItem(QStringLiteral("timeRange"), m_filteredAlerts.timeRange());
        }
        if (!m_filteredAlerts.statusFilter().isEmpty()) {
            query.addQueryItem(QStringLiteral("status"), m_filteredAlerts.statusFilter());
        }
        url.setQuery(query);

        const RestExport::Result result = RestExport::fetchJson(m_network, url);
        if (result.ok && !result.content.isEmpty()) {
            const QString filePath = exportFilePath("alerts");
            if (filePath.isEmpty()) {
                qWarning() << "exportAlerts failed to resolve export directory.";
                return {};
            }
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                qWarning() << "exportAlerts failed to open file:" << filePath;
                return {};
            }
            file.write(result.content);
            return filePath;
        }
    }

    // TODO: 接口未开发，当前仅导出本地 CSV 作为占位。
    const QString filePath = exportFilePath("alerts");
    if (filePath.isEmpty()) {
        qWarning() << "exportAlerts failed to resolve export directory.";
        return {};
    }
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "exportAlerts failed to open file:" << filePath;
        return {};
    }
    QTextStream out(&file);
    out << "Level,Message,Device,Time,Evidence,Action,Acknowledged,Muted\n";
    const int rows = m_filteredAlerts.rowCount();
    for (int row = 0; row < rows; ++row) {
        const QModelIndex idx = m_filteredAlerts.index(row, 0);
        const QString level = m_filteredAlerts.data(idx, AlertListModel::LevelRole).toString();
        const QString message = m_filteredAlerts.data(idx, AlertListModel::MessageRole).toString();
        const QString device = m_filteredAlerts.data(idx, AlertListModel::DeviceRole).toString();
        const QString time = m_filteredAlerts.data(idx, AlertListModel::TimeRole).toString();
        const QString evidence = m_filteredAlerts.data(idx, AlertListModel::EvidenceRole).toString();
        const QString action = m_filteredAlerts.data(idx, AlertListModel::ActionRole).toString();
        const bool acknowledged = m_filteredAlerts.data(idx, AlertListModel::AcknowledgedRole).toBool();
        const bool muted = m_filteredAlerts.data(idx, AlertListModel::MutedRole).toBool();

        out << csvEscape(level) << ","
            << csvEscape(message) << ","
            << csvEscape(device) << ","
            << csvEscape(time) << ","
            << csvEscape(evidence) << ","
            << csvEscape(action) << ","
            << (acknowledged ? "true" : "false") << ","
            << (muted ? "true" : "false") << "\n";
    }
    return filePath;
}

void AlertsViewModel::acknowledgeSelected()
{
    // TODO: 接口接入真实数据后仍需校验结果，当前调用模拟后端并本地标记状态。
    const QModelIndex proxyIndex = m_filteredAlerts.index(m_selectedIndex, 0);
    if (!proxyIndex.isValid()) {
        return;
    }
    const QModelIndex sourceIndex = m_filteredAlerts.mapToSource(proxyIndex);
    const AlertItem *item = m_alerts.itemAt(sourceIndex.row());
    if (!item) {
        return;
    }
    if (m_service) {
        m_service->acknowledgeAlert(item->id);
    }
    const QString key = alertKey(*item);
    m_statusByKey.insert(key, {true, item->muted});
    if (m_alerts.updateStatus(sourceIndex.row(), true, item->muted)) {
        emit selectedAlertChanged();
    }
}

void AlertsViewModel::muteSelected()
{
    // TODO: 接口接入真实数据后仍需校验结果，当前调用模拟后端并本地标记状态。
    const QModelIndex proxyIndex = m_filteredAlerts.index(m_selectedIndex, 0);
    if (!proxyIndex.isValid()) {
        return;
    }
    const QModelIndex sourceIndex = m_filteredAlerts.mapToSource(proxyIndex);
    const AlertItem *item = m_alerts.itemAt(sourceIndex.row());
    if (!item) {
        return;
    }
    if (m_service) {
        m_service->muteAlert(item->id);
    }
    const QString key = alertKey(*item);
    m_statusByKey.insert(key, {item->acknowledged, true});
    if (m_alerts.updateStatus(sourceIndex.row(), item->acknowledged, true)) {
        emit selectedAlertChanged();
    }
}

void AlertsViewModel::handleAlertsChanged(int topRow, int bottomRow)
{
    if (m_selectedIndex < 0) {
        return;
    }
    const QModelIndex proxyIndex = m_filteredAlerts.index(m_selectedIndex, 0);
    if (!proxyIndex.isValid()) {
        return;
    }
    const int sourceRow = m_filteredAlerts.mapToSource(proxyIndex).row();
    if (sourceRow >= topRow && sourceRow <= bottomRow) {
        emit selectedAlertChanged();
        updateEvidencePoints();
    }
}

void AlertsViewModel::refreshAlerts()
{
    if (!m_service) {
        return;
    }
    QVector<AlertItem> items = m_service->alerts();
    QSet<QString> seenKeys;
    for (AlertItem &item : items) {
        const QString key = alertKey(item);
        seenKeys.insert(key);
        if (m_statusByKey.contains(key)) {
            const auto status = m_statusByKey.value(key);
            item.acknowledged = status.first;
            item.muted = status.second;
        }
    }
    for (auto it = m_statusByKey.begin(); it != m_statusByKey.end();) {
        if (!seenKeys.contains(it.key())) {
            it = m_statusByKey.erase(it);
        } else {
            ++it;
        }
    }
    m_alerts.setItems(items);
    updateEvidencePoints();
}

void AlertsViewModel::ensureSelectionValid()
{
    const int count = m_filteredAlerts.rowCount();
    int nextIndex = m_selectedIndex;
    if (count == 0) {
        nextIndex = -1;
    } else if (m_selectedIndex < 0 || m_selectedIndex >= count) {
        nextIndex = 0;
    }

    if (nextIndex != m_selectedIndex) {
        m_selectedIndex = nextIndex;
        emit selectedIndexChanged();
    }
    emit selectedAlertChanged();
    updateEvidencePoints();
}

void AlertsViewModel::updateEvidencePoints()
{
    m_evidencePoints.clear();
    m_evidencePoints.reserve(16);

    const QString level = selectedAlert().value("level").toString();
    double base = 0.35;
    if (level == "Critical") {
        base = 0.7;
    } else if (level == "Warn") {
        base = 0.5;
    }

    for (int i = 0; i < 16; ++i) {
        const double jitter = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.4;
        const double value = qBound(0.05, base + jitter, 0.95);
        m_evidencePoints.append(value);
    }
    emit evidencePointsChanged();
}

QString AlertsViewModel::alertKey(const AlertItem &item) const
{
    if (!item.id.isEmpty()) {
        return item.id;
    }
    return item.device + "|" + item.time + "|" + item.level + "|" + item.message;
}
