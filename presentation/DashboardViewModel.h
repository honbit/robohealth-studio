#pragma once

#include "ViewModelBase.h"

#include <QTimer>
#include <QString>
#include <QVariantList>

class TelemetryServiceBase;

class DashboardViewModel : public ViewModelBase
{
    Q_OBJECT
    Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)
    Q_PROPERTY(double vibration READ vibration NOTIFY vibrationChanged)
    Q_PROPERTY(double current READ current NOTIFY currentChanged)
    Q_PROPERTY(double torque READ torque NOTIFY torqueChanged)
    Q_PROPERTY(double errorRate READ errorRate NOTIFY errorRateChanged)
    Q_PROPERTY(QVariantList trendPoints READ trendPoints NOTIFY trendPointsChanged)
    Q_PROPERTY(QString lastUpdated READ lastUpdated NOTIFY lastUpdatedChanged)

public:
    explicit DashboardViewModel(TelemetryServiceBase *service = nullptr, QObject *parent = nullptr);

    double temperature() const;
    double vibration() const;
    double current() const;
    double torque() const;
    double errorRate() const;
    QVariantList trendPoints() const;
    QString lastUpdated() const;

    Q_INVOKABLE void refresh();

signals:
    void temperatureChanged();
    void vibrationChanged();
    void currentChanged();
    void torqueChanged();
    void errorRateChanged();
    void trendPointsChanged();
    void lastUpdatedChanged();

private:
    void tick();
    void updateMetric(double &value, double min, double max, double step);
    void updateTrend(double normalizedValue);
    void updateLastUpdated();

    QTimer m_timer;
    TelemetryServiceBase *m_service = nullptr;
    double m_temperature = 36.4;
    double m_vibration = 0.42;
    double m_current = 12.8;
    double m_torque = 18.5;
    double m_errorRate = 0.8;
    QVariantList m_trendPoints;
    QString m_lastUpdated;
};
