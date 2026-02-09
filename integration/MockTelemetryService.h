#pragma once

#include "TelemetryServiceBase.h"

#include <QTimer>
#include <QVariantList>
#include <QVector>

class MockTelemetryService : public TelemetryServiceBase
{
    Q_OBJECT
public:
    explicit MockTelemetryService(QObject *parent = nullptr);

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
    void tick();
    void updateMetric(double &value, double min, double max, double step);
    void updateTrend(double normalizedValue);
    void updateDeviceHealth();
    void pushAlert();
    void pushLog();
    DeviceItem *findDevice(const QString &deviceName);

    QTimer m_timer;
    QVector<DeviceItem> m_devices;
    QVector<AlertItem> m_alerts;
    QVector<LogItem> m_logs;
    QVariantList m_trendPoints;
    double m_temperature = 36.4;
    double m_vibration = 0.42;
    double m_current = 12.8;
    double m_torque = 18.5;
    double m_errorRate = 0.8;
};
