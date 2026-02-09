#include "DashboardViewModel.h"

#include "TelemetryServiceBase.h"

#include <QRandomGenerator>
#include <QTime>
#include <QtGlobal>

DashboardViewModel::DashboardViewModel(TelemetryServiceBase *service, QObject *parent)
    : ViewModelBase(parent)
    , m_service(service)
{
    if (m_service) {
        connect(m_service, &TelemetryServiceBase::metricsUpdated, this, [this]() {
            updateLastUpdated();
            emit temperatureChanged();
            emit vibrationChanged();
            emit currentChanged();
            emit torqueChanged();
            emit errorRateChanged();
            emit trendPointsChanged();
        });
        updateLastUpdated();
    } else {
        m_trendPoints.reserve(20);
        for (int i = 0; i < 20; ++i) {
            m_trendPoints.append(0.2 + 0.02 * i);
        }

        m_timer.setInterval(1000);
        connect(&m_timer, &QTimer::timeout, this, &DashboardViewModel::tick);
        m_timer.start();
        updateLastUpdated();
    }
}

double DashboardViewModel::temperature() const
{
    return m_service ? m_service->temperature() : m_temperature;
}

double DashboardViewModel::vibration() const
{
    return m_service ? m_service->vibration() : m_vibration;
}

double DashboardViewModel::current() const
{
    return m_service ? m_service->current() : m_current;
}

double DashboardViewModel::torque() const
{
    return m_service ? m_service->torque() : m_torque;
}

double DashboardViewModel::errorRate() const
{
    return m_service ? m_service->errorRate() : m_errorRate;
}

QVariantList DashboardViewModel::trendPoints() const
{
    return m_service ? m_service->trendPoints() : m_trendPoints;
}

QString DashboardViewModel::lastUpdated() const
{
    return m_lastUpdated;
}

void DashboardViewModel::refresh()
{
    // TODO: 接口未开发，当前仅刷新展示状态。
    updateLastUpdated();
    emit temperatureChanged();
    emit vibrationChanged();
    emit currentChanged();
    emit torqueChanged();
    emit errorRateChanged();
    emit trendPointsChanged();
}

void DashboardViewModel::tick()
{
    if (m_service) {
        return;
    }
    updateMetric(m_temperature, 30.0, 60.0, 0.4);
    emit temperatureChanged();

    updateMetric(m_vibration, 0.1, 1.2, 0.05);
    emit vibrationChanged();

    updateMetric(m_current, 5.0, 20.0, 0.6);
    emit currentChanged();

    updateMetric(m_torque, 10.0, 30.0, 0.9);
    emit torqueChanged();

    updateMetric(m_errorRate, 0.0, 5.0, 0.2);
    emit errorRateChanged();

    double normalized = qBound(0.0, m_vibration / 1.2, 1.0);
    updateTrend(normalized);
}

void DashboardViewModel::updateMetric(double &value, double min, double max, double step)
{
    double delta = (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * step;
    value = qBound(min, value + delta, max);
}

void DashboardViewModel::updateTrend(double normalizedValue)
{
    if (m_trendPoints.size() < 20) {
        m_trendPoints.append(normalizedValue);
    } else {
        m_trendPoints.removeFirst();
        m_trendPoints.append(normalizedValue);
    }
    emit trendPointsChanged();
}

void DashboardViewModel::updateLastUpdated()
{
    m_lastUpdated = QTime::currentTime().toString("HH:mm:ss");
    emit lastUpdatedChanged();
}
