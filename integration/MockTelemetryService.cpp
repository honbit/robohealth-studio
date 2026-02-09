#include "MockTelemetryService.h"

#include <QRandomGenerator>
#include <QUuid>
#include <QTime>
#include <QtGlobal>

MockTelemetryService::MockTelemetryService(QObject *parent)
    : TelemetryServiceBase(parent)
{
    m_devices = {
        {"Robot-01", "Robot", "Online", "10:46", 92},
        {"Robot-02", "Robot", "Online", "10:45", 88},
        {"Cam-04", "Camera", "Offline", "09:50", 61},
        {"IMU-01", "IMU", "Online", "10:44", 95},
        {"PLC-01", "PLC", "Online", "10:40", 90}
    };

    m_alerts = {
        {"alert-1", "Critical", "Joint torque spike", "Robot-02", "10:42", "Torque > 20N*m", "Stop & inspect"},
        {"alert-2", "Warn", "Vibration above threshold", "Robot-01", "10:37", "Vibration > 0.4g", "Check bearing"},
        {"alert-3", "Info", "Camera reconnect", "Cam-04", "10:20", "Link restored", "None"}
    };

    m_logs = {
        {"Error", "Motor stall", "Robot-02", "10:43"},
        {"Warn", "Vibration high", "Robot-01", "10:37"},
        {"Info", "Camera reconnect", "Cam-04", "10:20"}
    };

    m_trendPoints.reserve(20);
    for (int i = 0; i < 20; ++i) {
        m_trendPoints.append(0.2 + 0.02 * i);
    }

    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &MockTelemetryService::tick);
    m_timer.start();
}

const QVector<DeviceItem> &MockTelemetryService::devices() const
{
    return m_devices;
}

const QVector<AlertItem> &MockTelemetryService::alerts() const
{
    return m_alerts;
}

const QVector<LogItem> &MockTelemetryService::logs() const
{
    return m_logs;
}

void MockTelemetryService::connectAll()
{
    const QString now = QTime::currentTime().toString("HH:mm");
    for (DeviceItem &device : m_devices) {
        device.status = "Online";
        device.lastSeen = now;
    }
    emit devicesUpdated();
}

void MockTelemetryService::disconnectAll()
{
    const QString now = QTime::currentTime().toString("HH:mm");
    for (DeviceItem &device : m_devices) {
        device.status = "Offline";
        device.lastSeen = now;
    }
    emit devicesUpdated();
}

void MockTelemetryService::disconnectDevice(const QString &deviceName)
{
    DeviceItem *device = findDevice(deviceName);
    if (!device) {
        return;
    }
    device->status = "Offline";
    device->lastSeen = QTime::currentTime().toString("HH:mm");
    emit devicesUpdated();
}

void MockTelemetryService::simulateFault(const QString &deviceName)
{
    if (m_devices.isEmpty()) {
        return;
    }

    DeviceItem *device = findDevice(deviceName);
    if (!device) {
        device = &m_devices[QRandomGenerator::global()->bounded(m_devices.size())];
    }

    const QString now = QTime::currentTime().toString("HH:mm");
    const int drop = QRandomGenerator::global()->bounded(8, 20);
    device->health = qBound(10, device->health - drop, 100);
    device->status = "Offline";
    device->lastSeen = now;
    emit devicesUpdated();

    AlertItem alert;
    alert.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    alert.level = "Critical";
    alert.message = "Vibration above threshold";
    alert.device = device->name;
    alert.time = now;
    alert.evidence = "Vibration > 0.4g";
    alert.action = "Stop & inspect";
    m_alerts.prepend(alert);
    if (m_alerts.size() > 20) {
        m_alerts.removeLast();
    }
    emit alertsUpdated();

    LogItem log;
    log.level = "Error";
    log.message = "Motor stall";
    log.device = device->name;
    log.time = now;
    m_logs.prepend(log);
    if (m_logs.size() > 30) {
        m_logs.removeLast();
    }
    emit logsUpdated();
}

bool MockTelemetryService::acknowledgeAlert(const QString &alertId)
{
    if (alertId.isEmpty()) {
        return false;
    }
    for (AlertItem &alert : m_alerts) {
        if (alert.id == alertId) {
            alert.acknowledged = true;
            emit alertsUpdated();
            return true;
        }
    }
    return false;
}

bool MockTelemetryService::muteAlert(const QString &alertId)
{
    if (alertId.isEmpty()) {
        return false;
    }
    for (AlertItem &alert : m_alerts) {
        if (alert.id == alertId) {
            alert.muted = true;
            emit alertsUpdated();
            return true;
        }
    }
    return false;
}

double MockTelemetryService::temperature() const
{
    return m_temperature;
}

double MockTelemetryService::vibration() const
{
    return m_vibration;
}

double MockTelemetryService::current() const
{
    return m_current;
}

double MockTelemetryService::torque() const
{
    return m_torque;
}

double MockTelemetryService::errorRate() const
{
    return m_errorRate;
}

QVariantList MockTelemetryService::trendPoints() const
{
    return m_trendPoints;
}

void MockTelemetryService::tick()
{
    updateMetric(m_temperature, 30.0, 60.0, 0.4);
    updateMetric(m_vibration, 0.1, 1.2, 0.05);
    updateMetric(m_current, 5.0, 20.0, 0.6);
    updateMetric(m_torque, 10.0, 30.0, 0.9);
    updateMetric(m_errorRate, 0.0, 5.0, 0.2);

    const double normalized = qBound(0.0, m_vibration / 1.2, 1.0);
    updateTrend(normalized);
    emit metricsUpdated();

    updateDeviceHealth();

    if (QRandomGenerator::global()->bounded(100) < 35) {
        pushLog();
    }

    if (QRandomGenerator::global()->bounded(100) < 20) {
        pushAlert();
    }
}

void MockTelemetryService::updateMetric(double &value, double min, double max, double step)
{
    const double delta = (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * step;
    value = qBound(min, value + delta, max);
}

void MockTelemetryService::updateTrend(double normalizedValue)
{
    if (m_trendPoints.size() < 20) {
        m_trendPoints.append(normalizedValue);
    } else {
        m_trendPoints.removeFirst();
        m_trendPoints.append(normalizedValue);
    }
}

void MockTelemetryService::updateDeviceHealth()
{
    if (m_devices.isEmpty()) {
        return;
    }

    const int row = QRandomGenerator::global()->bounded(m_devices.size());
    DeviceItem &device = m_devices[row];

    const int delta = QRandomGenerator::global()->bounded(-3, 4);
    device.health = qBound(50, device.health + delta, 100);
    device.lastSeen = QTime::currentTime().toString("HH:mm");

    if (QRandomGenerator::global()->bounded(100) < 5) {
        device.status = (device.status == "Online") ? "Offline" : "Online";
    }

    emit devicesUpdated();
}

void MockTelemetryService::pushAlert()
{
    if (m_devices.isEmpty()) {
        return;
    }

    struct Template {
        const char *level;
        const char *message;
        const char *evidence;
        const char *action;
    };

    const Template templates[] = {
        {"Critical", "Joint torque spike", "Torque > 20N*m", "Stop & inspect"},
        {"Warn", "Vibration above threshold", "Vibration > 0.4g", "Check bearing"},
        {"Info", "Camera reconnect", "Link restored", "None"}
    };

    const int templateIndex = QRandomGenerator::global()->bounded(0, 3);
    const int deviceIndex = QRandomGenerator::global()->bounded(0, m_devices.size());
    const DeviceItem &device = m_devices[deviceIndex];
    const Template &entry = templates[templateIndex];

    AlertItem item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.level = entry.level;
    item.message = entry.message;
    item.device = device.name;
    item.time = QTime::currentTime().toString("HH:mm");
    item.evidence = entry.evidence;
    item.action = entry.action;

    m_alerts.prepend(item);
    if (m_alerts.size() > 20) {
        m_alerts.removeLast();
    }

    emit alertsUpdated();
}

void MockTelemetryService::pushLog()
{
    if (m_devices.isEmpty()) {
        return;
    }

    struct Template {
        const char *level;
        const char *message;
    };

    const Template templates[] = {
        {"Error", "Motor stall"},
        {"Warn", "Vibration high"},
        {"Info", "Camera reconnect"}
    };

    const int templateIndex = QRandomGenerator::global()->bounded(0, 3);
    const int deviceIndex = QRandomGenerator::global()->bounded(0, m_devices.size());
    const DeviceItem &device = m_devices[deviceIndex];
    const Template &entry = templates[templateIndex];

    LogItem item;
    item.level = entry.level;
    item.message = entry.message;
    item.device = device.name;
    item.time = QTime::currentTime().toString("HH:mm");

    m_logs.prepend(item);
    if (m_logs.size() > 30) {
        m_logs.removeLast();
    }

    emit logsUpdated();
}

DeviceItem *MockTelemetryService::findDevice(const QString &deviceName)
{
    if (deviceName.isEmpty()) {
        return nullptr;
    }
    for (DeviceItem &device : m_devices) {
        if (device.name == deviceName) {
            return &device;
        }
    }
    return nullptr;
}
