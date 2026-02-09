#pragma once

#include "TelemetryServiceBase.h"

#include <QJsonObject>
#include <QHash>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QUrl>
#include <functional>

class RestTelemetryService : public TelemetryServiceBase
{
    Q_OBJECT
public:
    explicit RestTelemetryService(const QUrl &baseUrl, QObject *parent = nullptr);

    const QVector<DeviceItem> &devices() const override;
    const QVector<AlertItem> &alerts() const override;
    const QVector<LogItem> &logs() const override;

    void connectAll() override;
    void disconnectAll() override;
    void disconnectDevice(const QString &deviceName) override;
    void simulateFault(const QString &deviceName) override;
    bool acknowledgeAlert(const QString &alertId) override;
    bool muteAlert(const QString &alertId) override;

    double temperature() const override;
    double vibration() const override;
    double current() const override;
    double torque() const override;
    double errorRate() const override;
    QVariantList trendPoints() const override;

private:
    void poll();
    void pollDevices();
    void pollAlerts();
    void pollLogs();
    void pollMetrics();

    void getJson(const QString &path, std::function<void(const QJsonObject &)> handler);
    void postJson(const QString &path, const QJsonObject &payload = {});

    void handleDevices(const QJsonObject &payload);
    void handleAlerts(const QJsonObject &payload);
    void handleLogs(const QJsonObject &payload);
    void handleMetrics(const QJsonObject &payload);

    QString deviceIdForName(const QString &deviceName) const;

    QUrl m_baseUrl;
    QNetworkAccessManager m_network;
    QTimer m_pollTimer;
    int m_pollTick = 0;
    QVector<DeviceItem> m_devices;
    QVector<AlertItem> m_alerts;
    QVector<LogItem> m_logs;
    QVariantList m_trendPoints;
    double m_temperature = 0.0;
    double m_vibration = 0.0;
    double m_current = 0.0;
    double m_torque = 0.0;
    double m_errorRate = 0.0;
    QHash<QString, QString> m_deviceIdByName;
};
