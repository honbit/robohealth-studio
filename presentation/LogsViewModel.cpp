#include "LogsViewModel.h"

#include "RestExportHelper.h"
#include "TelemetryServiceBase.h"

#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include <QTime>
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

LogListModel::LogListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_items = {
        {"Error", "Motor stall", "Robot-02", "10:43"},
        {"Warn", "Vibration high", "Robot-01", "10:37"},
        {"Info", "Camera reconnect", "Cam-04", "10:20"}
    };
}

int LogListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.size();
}

QVariant LogListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return {};
    }
    const LogItem &item = m_items.at(index.row());
    switch (role) {
    case LevelRole: return item.level;
    case MessageRole: return item.message;
    case DeviceRole: return item.device;
    case TimeRole: return item.time;
    default: return {};
    }
}

QHash<int, QByteArray> LogListModel::roleNames() const
{
    return {
        {LevelRole, "level"},
        {MessageRole, "message"},
        {DeviceRole, "device"},
        {TimeRole, "time"}
    };
}

const LogItem *LogListModel::itemAt(int row) const
{
    if (row < 0 || row >= m_items.size()) {
        return nullptr;
    }
    return &m_items[row];
}

void LogListModel::setItems(const QVector<LogItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

LogsViewModel::LogsViewModel(TelemetryServiceBase *service, const QUrl &apiBase, bool useRest, QObject *parent)
    : ViewModelBase(parent)
    , m_service(service)
    , m_useRest(useRest)
    , m_apiBase(apiBase)
{
    if (m_useRest && !m_apiBase.isValid()) {
        m_apiBase = QUrl(QStringLiteral("http://127.0.0.1:5000"));
    }
    m_filteredLogs.setSourceModel(&m_logs);

    connect(&m_filteredLogs, &QAbstractItemModel::modelReset, this, &LogsViewModel::ensureSelectionValid);
    connect(&m_filteredLogs, &QAbstractItemModel::rowsInserted, this, &LogsViewModel::ensureSelectionValid);
    connect(&m_filteredLogs, &QAbstractItemModel::rowsRemoved, this, &LogsViewModel::ensureSelectionValid);
    connect(&m_filteredLogs, &QAbstractItemModel::layoutChanged, this, &LogsViewModel::ensureSelectionValid);

    connect(&m_logs, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
                handleLogsChanged(topLeft.row(), bottomRight.row());
            });

    if (m_service) {
        refreshLogs();
        connect(m_service, &TelemetryServiceBase::logsUpdated, this, &LogsViewModel::refreshLogs);
    }

    updateContextLines();
}

LogListModel *LogsViewModel::logs()
{
    return &m_logs;
}

LogFilterModel *LogsViewModel::filteredLogs()
{
    return &m_filteredLogs;
}

int LogsViewModel::selectedIndex() const
{
    return m_selectedIndex;
}

void LogsViewModel::setSelectedIndex(int index)
{
    if (m_selectedIndex == index) {
        return;
    }
    m_selectedIndex = index;
    emit selectedIndexChanged();
    emit selectedLogChanged();
    updateContextLines();
}

QVariantMap LogsViewModel::selectedLog() const
{
    QVariantMap map;
    const QModelIndex proxyIndex = m_filteredLogs.index(m_selectedIndex, 0);
    if (!proxyIndex.isValid()) {
        return map;
    }
    const QModelIndex sourceIndex = m_filteredLogs.mapToSource(proxyIndex);
    const LogItem *item = m_logs.itemAt(sourceIndex.row());
    if (!item) {
        return map;
    }
    map.insert("level", item->level);
    map.insert("message", item->message);
    map.insert("device", item->device);
    map.insert("time", item->time);
    return map;
}

QVariantList LogsViewModel::contextLines() const
{
    return m_contextLines;
}

QString LogsViewModel::exportLogs()
{
    if (m_useRest && m_apiBase.isValid()) {
        QUrl url = RestExport::resolveApiUrl(m_apiBase, QStringLiteral("/api/exports/logs"));
        QUrlQuery query;
        if (!m_filteredLogs.searchText().isEmpty()) {
            query.addQueryItem(QStringLiteral("search"), m_filteredLogs.searchText());
        }
        if (!m_filteredLogs.levelFilter().isEmpty()) {
            query.addQueryItem(QStringLiteral("level"), m_filteredLogs.levelFilter());
        }
        if (!m_filteredLogs.timeRange().isEmpty()) {
            query.addQueryItem(QStringLiteral("timeRange"), m_filteredLogs.timeRange());
        }
        url.setQuery(query);

        const RestExport::Result result = RestExport::fetchJson(m_network, url);
        if (result.ok && !result.content.isEmpty()) {
            const QString filePath = exportFilePath("logs");
            if (filePath.isEmpty()) {
                qWarning() << "exportLogs failed to resolve export directory.";
                return {};
            }
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                qWarning() << "exportLogs failed to open file:" << filePath;
                return {};
            }
            file.write(result.content);
            return filePath;
        }
    }

    // TODO: 接口未开发，当前仅导出本地 CSV 作为占位。
    const QString filePath = exportFilePath("logs");
    if (filePath.isEmpty()) {
        qWarning() << "exportLogs failed to resolve export directory.";
        return {};
    }
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "exportLogs failed to open file:" << filePath;
        return {};
    }
    QTextStream out(&file);
    out << "Level,Message,Device,Time\n";
    const int rows = m_filteredLogs.rowCount();
    for (int row = 0; row < rows; ++row) {
        const QModelIndex idx = m_filteredLogs.index(row, 0);
        const QString level = m_filteredLogs.data(idx, LogListModel::LevelRole).toString();
        const QString message = m_filteredLogs.data(idx, LogListModel::MessageRole).toString();
        const QString device = m_filteredLogs.data(idx, LogListModel::DeviceRole).toString();
        const QString time = m_filteredLogs.data(idx, LogListModel::TimeRole).toString();

        out << csvEscape(level) << ","
            << csvEscape(message) << ","
            << csvEscape(device) << ","
            << csvEscape(time) << "\n";
    }
    return filePath;
}

void LogsViewModel::handleLogsChanged(int topRow, int bottomRow)
{
    if (m_selectedIndex < 0) {
        return;
    }
    const QModelIndex proxyIndex = m_filteredLogs.index(m_selectedIndex, 0);
    if (!proxyIndex.isValid()) {
        return;
    }
    const int sourceRow = m_filteredLogs.mapToSource(proxyIndex).row();
    if (sourceRow >= topRow && sourceRow <= bottomRow) {
        emit selectedLogChanged();
        updateContextLines();
    }
}

void LogsViewModel::refreshLogs()
{
    if (!m_service) {
        return;
    }
    m_logs.setItems(m_service->logs());
    updateContextLines();
}

void LogsViewModel::ensureSelectionValid()
{
    const int count = m_filteredLogs.rowCount();
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
    emit selectedLogChanged();
    updateContextLines();
}

void LogsViewModel::updateContextLines()
{
    m_contextLines.clear();
    const QVariantMap log = selectedLog();
    if (log.isEmpty()) {
        emit contextLinesChanged();
        return;
    }

    QStringList texts;
    const QString message = log.value("message").toString();
    if (!message.isEmpty()) {
        texts << message;
    }
    texts << "Current surge";
    texts << "Temperature rise";
    texts.removeDuplicates();

    QTime baseTime = QTime::fromString(log.value("time").toString(), "HH:mm");
    if (!baseTime.isValid()) {
        baseTime = QTime::currentTime();
    }

    const int count = texts.size();
    for (int i = 0; i < count; ++i) {
        QTime lineTime = baseTime.addSecs((i - (count - 1)) * 120);
        QVariantMap line;
        line.insert("time", lineTime.toString("HH:mm"));
        line.insert("text", texts.at(i));
        m_contextLines.append(line);
    }
    emit contextLinesChanged();
}
