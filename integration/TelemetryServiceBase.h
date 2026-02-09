#pragma once

#include "TelemetryItems.h"

#include <QObject>
#include <QVariantList>
#include <QVector>

class TelemetryServiceBase : public QObject
{
    Q_OBJECT
public:
    explicit TelemetryServiceBase(QObject *parent = nullptr) : QObject(parent) {}
    ~TelemetryServiceBase() override = default;

    virtual const QVector<DeviceItem> &devices() const = 0;
    virtual const QVector<AlertItem> &alerts() const = 0;
    virtual const QVector<LogItem> &logs() const = 0;

    virtual void connectAll() = 0;
    virtual void disconnectAll() = 0;
    virtual void disconnectDevice(const QString &deviceName) = 0;
    virtual void simulateFault(const QString &deviceName) = 0;
    virtual bool acknowledgeAlert(const QString &alertId) = 0;
    virtual bool muteAlert(const QString &alertId) = 0;

    virtual double temperature() const = 0;
    virtual double vibration() const = 0;
    virtual double current() const = 0;
    virtual double torque() const = 0;
    virtual double errorRate() const = 0;
    virtual QVariantList trendPoints() const = 0;

signals:
    void devicesUpdated();
    void alertsUpdated();
    void logsUpdated();
    void metricsUpdated();
};
