#include "ReportsViewModel.h"

#include "RestExportHelper.h"
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QScopedPointer>
#include <QStandardPaths>
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

QString exportFilePath(const QString &baseName, const QString &ext)
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
    return dir.filePath(exportDirName + "/" + baseName + "_" + timestamp + "." + ext);
}

QUrl resolveUrl(const QUrl &baseUrl, const QString &path)
{
    QUrl resolved = baseUrl;
    if (!resolved.path().endsWith('/')) {
        resolved.setPath(resolved.path() + '/');
    }
    return resolved.resolved(QUrl(path.startsWith('/') ? path.mid(1) : path));
}
}

QVector<ReportItem> parseReports(const QJsonObject &payload)
{
    QVector<ReportItem> reports;
    const QJsonArray items = payload.value("items").toArray();
    reports.reserve(items.size());
    for (const QJsonValue &value : items) {
        const QJsonObject item = value.toObject();
        ReportItem report;
        report.title = item.value("title").toString();
        report.time = item.value("time").toString();
        report.summary = item.value("summary").toString();
        reports.push_back(report);
    }
    return reports;
}

ReportListModel::ReportListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ReportListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_reports.size();
}

QVariant ReportListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_reports.size()) {
        return {};
    }

    const ReportItem &item = m_reports.at(index.row());
    switch (role) {
    case TitleRole: return item.title;
    case TimeRole: return item.time;
    case SummaryRole: return item.summary;
    default: return {};
    }
}

QHash<int, QByteArray> ReportListModel::roleNames() const
{
    return {
        {TitleRole, "title"},
        {TimeRole, "time"},
        {SummaryRole, "summary"}
    };
}

void ReportListModel::setReports(const QVector<ReportItem> &reports)
{
    beginResetModel();
    m_reports = reports;
    endResetModel();
}

const ReportItem *ReportListModel::itemAt(int row) const
{
    if (row < 0 || row >= m_reports.size()) {
        return nullptr;
    }
    return &m_reports[row];
}

ReportsViewModel::ReportsViewModel(const QUrl &apiBase, bool useRest, QObject *parent)
    : ViewModelBase(parent)
    , m_useRest(useRest)
    , m_apiBase(apiBase)
{
    if (m_useRest && !m_apiBase.isValid()) {
        m_apiBase = QUrl(QStringLiteral("http://127.0.0.1:5000"));
    }
    refreshReports();
}

ReportListModel *ReportsViewModel::reports()
{
    return &m_reports;
}

QString ReportsViewModel::timeRange() const
{
    return m_timeRange;
}

QString ReportsViewModel::reportType() const
{
    return m_reportType;
}

QString ReportsViewModel::deviceFilter() const
{
    return m_deviceFilter;
}

void ReportsViewModel::setTimeRange(const QString &value)
{
    QString normalized = value;
    if (normalized != "1h" && normalized != "24h" && normalized != "7d") {
        normalized = "24h";
    }
    if (m_timeRange == normalized) {
        return;
    }
    m_timeRange = normalized;
    emit timeRangeChanged();
    refreshReports();
}

void ReportsViewModel::setReportType(const QString &value)
{
    const QString normalized = value.isEmpty() ? "Daily" : value;
    if (m_reportType == normalized) {
        return;
    }
    m_reportType = normalized;
    emit reportTypeChanged();
    refreshReports();
}

void ReportsViewModel::setDeviceFilter(const QString &value)
{
    if (m_deviceFilter == value) {
        return;
    }
    m_deviceFilter = value;
    emit deviceFilterChanged();
    refreshReports();
}

int ReportsViewModel::selectedIndex() const
{
    return m_selectedIndex;
}

void ReportsViewModel::setSelectedIndex(int index)
{
    if (m_selectedIndex == index) {
        return;
    }
    m_selectedIndex = index;
    emit selectedIndexChanged();
    emit selectedReportChanged();
    updatePreview();
}

QVariantMap ReportsViewModel::selectedReport() const
{
    QVariantMap map;
    const ReportItem *item = m_reports.itemAt(m_selectedIndex);
    if (!item) {
        return map;
    }
    map.insert("title", item->title);
    map.insert("time", item->time);
    map.insert("summary", item->summary);
    return map;
}

QVariantList ReportsViewModel::previewMetrics() const
{
    return m_previewMetrics;
}

QVariantList ReportsViewModel::previewHighlights() const
{
    return m_previewHighlights;
}

void ReportsViewModel::preview()
{
    ensureSelectionValid();
}

QString ReportsViewModel::exportReport()
{
    if (m_useRest && m_apiBase.isValid()) {
        QUrl url = RestExport::resolveApiUrl(m_apiBase, QStringLiteral("/api/exports/reports"));
        QUrlQuery query;
        if (!m_timeRange.isEmpty()) {
            query.addQueryItem(QStringLiteral("timeRange"), m_timeRange);
        }
        if (!m_reportType.isEmpty()) {
            query.addQueryItem(QStringLiteral("reportType"), m_reportType);
        }
        if (!m_deviceFilter.isEmpty()) {
            query.addQueryItem(QStringLiteral("device"), m_deviceFilter);
        }
        url.setQuery(query);

        const RestExport::Result result = RestExport::fetchJson(m_network, url);
        if (result.ok && !result.content.isEmpty()) {
            const QString filePath = exportFilePath("reports", "csv");
            if (filePath.isEmpty()) {
                return {};
            }
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                return {};
            }
            file.write(result.content);
            return filePath;
        }
    }

    // TODO: 接口未开发，当前仅导出本地 CSV 作为占位。
    const QString filePath = exportFilePath("reports", "csv");
    if (filePath.isEmpty()) {
        return {};
    }
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return {};
    }
    QTextStream out(&file);
    out << "Title,Time,Summary,ReportType,TimeRange,Device\n";
    const int rows = m_reports.rowCount();
    for (int row = 0; row < rows; ++row) {
        const QModelIndex idx = m_reports.index(row, 0);
        const QString title = m_reports.data(idx, ReportListModel::TitleRole).toString();
        const QString time = m_reports.data(idx, ReportListModel::TimeRole).toString();
        const QString summary = m_reports.data(idx, ReportListModel::SummaryRole).toString();
        out << csvEscape(title) << ","
            << csvEscape(time) << ","
            << csvEscape(summary) << ","
            << csvEscape(m_reportType) << ","
            << csvEscape(m_timeRange) << ","
            << csvEscape(m_deviceFilter) << "\n";
    }
    return filePath;
}

QString ReportsViewModel::exportPdf()
{
    const ReportItem *item = m_reports.itemAt(m_selectedIndex);
    if (!item) {
        return {};
    }
    if (m_useRest && m_apiBase.isValid()) {
        QUrl url = RestExport::resolveApiUrl(m_apiBase, QStringLiteral("/api/exports/reportPdf"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("title"), item->title);
        query.addQueryItem(QStringLiteral("time"), item->time);
        query.addQueryItem(QStringLiteral("summary"), item->summary);
        if (!m_reportType.isEmpty()) {
            query.addQueryItem(QStringLiteral("reportType"), m_reportType);
        }
        if (!m_timeRange.isEmpty()) {
            query.addQueryItem(QStringLiteral("timeRange"), m_timeRange);
        }
        if (!m_deviceFilter.isEmpty()) {
            query.addQueryItem(QStringLiteral("device"), m_deviceFilter);
        }
        url.setQuery(query);

        const RestExport::Result result = RestExport::fetchJson(m_network, url);
        if (result.ok && !result.content.isEmpty()) {
            const QString filePath = exportFilePath("report", "pdf");
            if (filePath.isEmpty()) {
                return {};
            }
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                return {};
            }
            file.write(result.content);
            return filePath;
        }
    }

    // TODO: 接口未开发，当前仅导出本地占位文件。
    const QString filePath = exportFilePath("report", "pdf");
    if (filePath.isEmpty()) {
        return {};
    }
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return {};
    }
    QTextStream out(&file);
    out << "RoboHealth Report (mock PDF)\n";
    out << "Title: " << item->title << "\n";
    out << "Time: " << item->time << "\n";
    out << "Summary: " << item->summary << "\n";
    out << "ReportType: " << m_reportType << "\n";
    out << "TimeRange: " << m_timeRange << "\n";
    out << "Device: " << (m_deviceFilter.isEmpty() ? "All" : m_deviceFilter) << "\n";
    return filePath;
}

void ReportsViewModel::ensureSelectionValid()
{
    const int count = m_reports.rowCount();
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
    emit selectedReportChanged();
    updatePreview();
}

void ReportsViewModel::refreshReports()
{
    if (m_useRest && m_apiBase.isValid()) {
        fetchReports();
        return;
    }
    buildLocalReports();
}

void ReportsViewModel::buildLocalReports()
{
    const QDate today = QDate::currentDate();
    QVector<ReportItem> reports;
    QString timeLabel;
    if (m_reportType == "Monthly") {
        timeLabel = today.toString("yyyy-MM");
    } else if (m_timeRange == "7d") {
        timeLabel = today.addDays(-6).toString("yyyy-MM-dd") + " ~ " + today.toString("yyyy-MM-dd");
    } else {
        timeLabel = today.toString("yyyy-MM-dd");
    }
    if (!m_deviceFilter.isEmpty()) {
        timeLabel += " · " + m_deviceFilter;
    }

    if (m_reportType == "Monthly") {
        reports.push_back({"Monthly Health Summary", timeLabel, "Monthly health scores and uptime summary."});
        reports.push_back({"Daily Alerts", timeLabel, "Daily alert counts and critical incidents."});
    } else if (m_reportType == "Weekly") {
        reports.push_back({"Weekly Health Summary", timeLabel, "Weekly health scores and uptime summary."});
        reports.push_back({"Daily Alerts", timeLabel, "Daily alert counts and critical incidents."});
    } else if (m_timeRange == "1h") {
        reports.push_back({"Daily Alerts", timeLabel, "Daily alert counts and critical incidents."});
        reports.push_back({"Hourly Snapshot", timeLabel, "Hourly anomalies and key metrics."});
    } else if (m_reportType == "Daily") {
        reports.push_back({"Daily Alerts", timeLabel, "Daily alert counts and critical incidents."});
        reports.push_back({"Daily Health Summary", timeLabel, "Daily health scores and uptime summary."});
    } else {
        reports.push_back({"Weekly Health Summary", timeLabel, "Weekly health scores and uptime summary."});
        reports.push_back({"Daily Alerts", timeLabel, "Daily alert counts and critical incidents."});
    }

    m_reports.setReports(reports);
    ensureSelectionValid();
}

void ReportsViewModel::fetchReports()
{
    const int requestId = ++m_requestId;
    QUrl url = resolveUrl(m_apiBase, "/api/reports");
    QUrlQuery query;
    query.addQueryItem("timeRange", m_timeRange);
    query.addQueryItem("reportType", m_reportType);
    if (!m_deviceFilter.isEmpty()) {
        query.addQueryItem("device", m_deviceFilter);
    }
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, requestId]() {
        const QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
        if (requestId != m_requestId) {
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            buildLocalReports();
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isObject()) {
            buildLocalReports();
            return;
        }
        QVector<ReportItem> reports = parseReports(doc.object());
        if (reports.isEmpty()) {
            buildLocalReports();
            return;
        }
        m_reports.setReports(reports);
        ensureSelectionValid();
    });
}

void ReportsViewModel::updatePreview()
{
    m_previewMetrics = buildMetrics();
    m_previewHighlights = buildHighlights();
    emit previewMetricsChanged();
    emit previewHighlightsChanged();
}

QVariantList ReportsViewModel::buildMetrics() const
{
    QVariantList metrics;
    const ReportItem *item = m_reports.itemAt(m_selectedIndex);
    if (!item) {
        return metrics;
    }

    int alertCount = 12;
    int criticalCount = 2;
    int avgHealth = 92;
    double uptime = 98.6;

    if (m_reportType == "Weekly") {
        alertCount = 48;
        criticalCount = 6;
        avgHealth = 90;
        uptime = 97.8;
    } else if (m_reportType == "Monthly") {
        alertCount = 160;
        criticalCount = 14;
        avgHealth = 88;
        uptime = 96.9;
    } else if (m_timeRange == "1h") {
        alertCount = 3;
        criticalCount = 1;
        avgHealth = 95;
        uptime = 99.2;
    }

    auto pushMetric = [&](const QString &label, const QString &value, const QString &unit, bool alerted) {
        QVariantMap metric;
        metric.insert("label", label);
        metric.insert("value", value);
        metric.insert("unit", unit);
        metric.insert("alerted", alerted);
        metrics.append(metric);
    };

    pushMetric("Alert count", QString::number(alertCount), "", alertCount >= 50);
    pushMetric("Critical incidents", QString::number(criticalCount), "", criticalCount >= 5);
    pushMetric("Avg health", QString::number(avgHealth), "%", avgHealth < 90);
    pushMetric("Uptime", QString::number(uptime, 'f', 1), "%", uptime < 97.0);
    return metrics;
}

QVariantList ReportsViewModel::buildHighlights() const
{
    QVariantList highlights;
    const ReportItem *item = m_reports.itemAt(m_selectedIndex);
    if (!item) {
        return highlights;
    }

    if (m_reportType == "Monthly") {
        highlights.append("Top issue: vibration spikes on Robot-02");
        highlights.append("Critical incidents concentrated in late shifts");
        highlights.append("Recommended action: replace worn bearings");
    } else if (m_reportType == "Weekly") {
        highlights.append("Alerts stabilized after mid-week maintenance");
        highlights.append("Robot-01 shows improving health score");
        highlights.append("Top anomaly: current surge during peak loads");
    } else {
        highlights.append("Alerts peaked around 10:30");
        highlights.append("Camera reconnect events trending down");
        highlights.append("Critical events are isolated");
    }
    return highlights;
}
