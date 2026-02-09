#pragma once

#include "ViewModelBase.h"

#include <QString>

class SettingsViewModel : public ViewModelBase
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QString timeRange READ timeRange WRITE setTimeRange NOTIFY timeRangeChanged)
    Q_PROPERTY(bool autoRefresh READ autoRefresh WRITE setAutoRefresh NOTIFY autoRefreshChanged)
    Q_PROPERTY(int deviceCount READ deviceCount WRITE setDeviceCount NOTIFY deviceCountChanged)
    Q_PROPERTY(double faultRate READ faultRate WRITE setFaultRate NOTIFY faultRateChanged)
    Q_PROPERTY(int refreshHz READ refreshHz WRITE setRefreshHz NOTIFY refreshHzChanged)
    Q_PROPERTY(double vibrationThreshold READ vibrationThreshold WRITE setVibrationThreshold NOTIFY vibrationThresholdChanged)
    Q_PROPERTY(double torqueThreshold READ torqueThreshold WRITE setTorqueThreshold NOTIFY torqueThresholdChanged)
    Q_PROPERTY(double currentThreshold READ currentThreshold WRITE setCurrentThreshold NOTIFY currentThresholdChanged)

public:
    explicit SettingsViewModel(QObject *parent = nullptr);

    QString language() const;
    QString theme() const;
    QString timeRange() const;
    bool autoRefresh() const;
    int deviceCount() const;
    double faultRate() const;
    int refreshHz() const;
    double vibrationThreshold() const;
    double torqueThreshold() const;
    double currentThreshold() const;

    void setLanguage(const QString &value);
    void setTheme(const QString &value);
    void setTimeRange(const QString &value);
    void setAutoRefresh(bool value);
    void setDeviceCount(int value);
    void setFaultRate(double value);
    void setRefreshHz(int value);
    void setVibrationThreshold(double value);
    void setTorqueThreshold(double value);
    void setCurrentThreshold(double value);

signals:
    void languageChanged();
    void themeChanged();
    void timeRangeChanged();
    void autoRefreshChanged();
    void deviceCountChanged();
    void faultRateChanged();
    void refreshHzChanged();
    void vibrationThresholdChanged();
    void torqueThresholdChanged();
    void currentThresholdChanged();

private:
    void loadSettings();
    void saveSetting(const QString &key, const QVariant &value);

    QString m_language = "zh";
    QString m_theme = "light";
    QString m_timeRange = "24h";
    bool m_autoRefresh = true;
    int m_deviceCount = 20;
    double m_faultRate = 0.2;
    int m_refreshHz = 2;
    double m_vibrationThreshold = 0.40;
    double m_torqueThreshold = 20.0;
    double m_currentThreshold = 15.0;
};
