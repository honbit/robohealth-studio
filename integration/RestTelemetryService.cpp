#include "RestTelemetryService.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QScopedPointer>

namespace {
QUrl resolveUrl(const QUrl &baseUrl, const QString &path)
{
    QUrl resolved = baseUrl;
    if (!resolved.path().endsWith('/')) {
        resolved.setPath(resolved.path() + '/');
    }
    return resolved.resolved(QUrl(path.startsWith('/') ? path.mid(1) : path));
}

QString toStringSafe(const QJsonValue &value)
{
    return value.isString() ? value.toString() : QString();
}
}

RestTelemetryService::RestTelemetryService(const QUrl &baseUrl, QObject *parent)
    : TelemetryServiceBase(parent)
    , m_baseUrl(baseUrl)
{
    if (!m_baseUrl.isValid()) {
        m_baseUrl = QUrl(QStringLiteral("http://127.0.0.1:5000"));
    }

    m_pollTimer.setInterval(1000);
    connect(&m_pollTimer, &QTimer::timeout, this, &RestTelemetryService::poll);
    m_pollTimer.start();

    pollDevices();
    pollAlerts();
    pollLogs();
    pollMetrics();
}

const QVector<DeviceItem> &RestTelemetryService::devices() const
{
    return m_devices;
}

const QVector<AlertItem> &RestTelemetryService::alerts() const
{
    return m_alerts;
}

const QVector<LogItem> &RestTelemetryService::logs() const
{
    return m_logs;
}

void RestTelemetryService::connectAll()
{
    postJson("/api/devices/connectAll");
}

void RestTelemetryService::disconnectAll()
{
    postJson("/api/devices/disconnectAll");
}

void RestTelemetryService::disconnectDevice(const QString &deviceName)
{
    const QString id = deviceIdForName(deviceName);
    if (id.isEmpty()) {
        return;
    }
    postJson(QString("/api/devices/%1/disconnect").arg(id));
}

void RestTelemetryService::simulateFault(const QString &deviceName)
{
    const QString id = deviceIdForName(deviceName);
    if (id.isEmpty()) {
        return;
    }
    postJson(QString("/api/devices/%1/simulateFault").arg(id));
}

bool RestTelemetryService::acknowledgeAlert(const QString &alertId)
{
    if (alertId.isEmpty()) {
        return false;
    }
    postJson(QString("/api/alerts/%1/ack").arg(alertId));
    return true;
}

bool RestTelemetryService::muteAlert(const QString &alertId)
{
    if (alertId.isEmpty()) {
        return false;
    }
    postJson(QString("/api/alerts/%1/mute").arg(alertId));
    return true;
}

double RestTelemetryService::temperature() const
{
    return m_temperature;
}

double RestTelemetryService::vibration() const
{
    return m_vibration;
}

double RestTelemetryService::current() const
{
    return m_current;
}

double RestTelemetryService::torque() const
{
    return m_torque;
}

double RestTelemetryService::errorRate() const
{
    return m_errorRate;
}

QVariantList RestTelemetryService::trendPoints() const
{
    return m_trendPoints;
}

void RestTelemetryService::poll()
{
    m_pollTick += 1;
    pollMetrics();
    pollLogs();
    pollAlerts();

    if (m_pollTick % 3 == 0) {
        pollDevices();
    }
}

void RestTelemetryService::pollDevices()
{
    getJson("/api/devices", [this](const QJsonObject &payload) {
        handleDevices(payload);
    });
}

void RestTelemetryService::pollAlerts()
{
    getJson("/api/alerts?limit=20", [this](const QJsonObject &payload) {
        handleAlerts(payload);
    });
}

void RestTelemetryService::pollLogs()
{
    getJson("/api/logs?limit=30", [this](const QJsonObject &payload) {
        handleLogs(payload);
    });
}

void RestTelemetryService::pollMetrics()
{
    getJson("/api/metrics", [this](const QJsonObject &payload) {
        handleMetrics(payload);
    });
}

void RestTelemetryService::getJson(const QString &path, std::function<void(const QJsonObject &)> handler)
{
    QNetworkRequest request(resolveUrl(m_baseUrl, path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [reply, handler]() {
        const QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
        if (reply->error() != QNetworkReply::NoError) {
            return;
        }
        const QByteArray data = reply->readAll();
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            return;
        }
        handler(doc.object());
    });
}

void RestTelemetryService::postJson(const QString &path, const QJsonObject &payload)
{
    QNetworkRequest request(resolveUrl(m_baseUrl, path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const QJsonDocument doc(payload);
    QNetworkReply *reply = m_network.post(request, doc.toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [reply]() {
        reply->deleteLater();
    });
}

void RestTelemetryService::handleDevices(const QJsonObject &payload)
{
    const QJsonArray items = payload.value("items").toArray();
    QVector<DeviceItem> devices;
    devices.reserve(items.size());
    QHash<QString, QString> idByName;

    for (const QJsonValue &value : items) {
        const QJsonObject item = value.toObject();
        DeviceItem device;
        device.name = toStringSafe(item.value("name"));
        device.type = toStringSafe(item.value("type"));
        device.status = toStringSafe(item.value("status"));
        device.lastSeen = toStringSafe(item.value("lastSeen"));
        device.health = item.value("health").toInt();
        devices.append(device);

        const QString id = toStringSafe(item.value("id"));
        if (!id.isEmpty() && !device.name.isEmpty()) {
            idByName.insert(device.name, id);
        }
    }

    m_deviceIdByName = idByName;
    m_devices = devices;
    emit devicesUpdated();
}

void RestTelemetryService::handleAlerts(const QJsonObject &payload)
{
    const QJsonArray items = payload.value("items").toArray();
    QVector<AlertItem> alerts;
    alerts.reserve(items.size());

    for (const QJsonValue &value : items) {
        const QJsonObject item = value.toObject();
        AlertItem alert;
        alert.id = toStringSafe(item.value("id"));
        alert.level = toStringSafe(item.value("level"));
        alert.message = toStringSafe(item.value("message"));
        alert.device = toStringSafe(item.value("deviceName"));
        if (alert.device.isEmpty()) {
            alert.device = toStringSafe(item.value("device"));
        }
        alert.time = toStringSafe(item.value("time"));
        alert.evidence = toStringSafe(item.value("evidence"));
        alert.action = toStringSafe(item.value("action"));
        alert.acknowledged = item.value("acknowledged").toBool();
        alert.muted = item.value("muted").toBool();
        alerts.append(alert);
    }

    m_alerts = alerts;
    emit alertsUpdated();
}

void RestTelemetryService::handleLogs(const QJsonObject &payload)
{
    const QJsonArray items = payload.value("items").toArray();
    QVector<LogItem> logs;
    logs.reserve(items.size());

    for (const QJsonValue &value : items) {
        const QJsonObject item = value.toObject();
        LogItem log;
        log.level = toStringSafe(item.value("level"));
        log.message = toStringSafe(item.value("message"));
        log.device = toStringSafe(item.value("deviceName"));
        if (log.device.isEmpty()) {
            log.device = toStringSafe(item.value("device"));
        }
        log.time = toStringSafe(item.value("time"));
        logs.append(log);
    }

    m_logs = logs;
    emit logsUpdated();
}

void RestTelemetryService::handleMetrics(const QJsonObject &payload)
{
    m_temperature = payload.value("temperature").toDouble(m_temperature);
    m_vibration = payload.value("vibration").toDouble(m_vibration);
    m_current = payload.value("current").toDouble(m_current);
    m_torque = payload.value("torque").toDouble(m_torque);
    m_errorRate = payload.value("errorRate").toDouble(m_errorRate);

    QVariantList points;
    const QJsonArray trend = payload.value("trendPoints").toArray();
    points.reserve(trend.size());
    for (const QJsonValue &value : trend) {
        points.append(value.toDouble());
    }
    if (!trend.isEmpty()) {
        m_trendPoints = points;
    }
    emit metricsUpdated();
}

QString RestTelemetryService::deviceIdForName(const QString &deviceName) const
{
    return m_deviceIdByName.value(deviceName);
}
