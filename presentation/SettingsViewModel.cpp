#include "SettingsViewModel.h"

#include <QSettings>
#include <QtGlobal>

namespace {
constexpr char kLanguageKey[] = "settings/language";
constexpr char kThemeKey[] = "settings/theme";
constexpr char kTimeRangeKey[] = "settings/timeRange";
constexpr char kAutoRefreshKey[] = "settings/autoRefresh";
constexpr char kDeviceCountKey[] = "settings/deviceCount";
constexpr char kFaultRateKey[] = "settings/faultRate";
constexpr char kRefreshHzKey[] = "settings/refreshHz";
constexpr char kVibrationThresholdKey[] = "settings/vibrationThreshold";
constexpr char kTorqueThresholdKey[] = "settings/torqueThreshold";
constexpr char kCurrentThresholdKey[] = "settings/currentThreshold";

QString normalizeTheme(const QString &value)
{
    return value.toLower() == "dark" ? "dark" : "light";
}

QString normalizeTimeRange(const QString &value)
{
    if (value == "1h" || value == "24h" || value == "7d") {
        return value;
    }
    return "24h";
}
}

SettingsViewModel::SettingsViewModel(QObject *parent)
    : ViewModelBase(parent)
{
    loadSettings();
}

QString SettingsViewModel::language() const
{
    return m_language;
}

QString SettingsViewModel::theme() const
{
    return m_theme;
}

QString SettingsViewModel::timeRange() const
{
    return m_timeRange;
}

bool SettingsViewModel::autoRefresh() const
{
    return m_autoRefresh;
}

int SettingsViewModel::deviceCount() const
{
    return m_deviceCount;
}

double SettingsViewModel::faultRate() const
{
    return m_faultRate;
}

int SettingsViewModel::refreshHz() const
{
    return m_refreshHz;
}

double SettingsViewModel::vibrationThreshold() const
{
    return m_vibrationThreshold;
}

double SettingsViewModel::torqueThreshold() const
{
    return m_torqueThreshold;
}

double SettingsViewModel::currentThreshold() const
{
    return m_currentThreshold;
}

void SettingsViewModel::setLanguage(const QString &value)
{
    if (m_language == value) {
        return;
    }
    m_language = value;
    emit languageChanged();
    saveSetting(kLanguageKey, m_language);
}

void SettingsViewModel::setTheme(const QString &value)
{
    const QString normalized = normalizeTheme(value);
    if (m_theme == normalized) {
        return;
    }
    m_theme = normalized;
    emit themeChanged();
    saveSetting(kThemeKey, m_theme);
}

void SettingsViewModel::setTimeRange(const QString &value)
{
    const QString normalized = normalizeTimeRange(value);
    if (m_timeRange == normalized) {
        return;
    }
    m_timeRange = normalized;
    emit timeRangeChanged();
    saveSetting(kTimeRangeKey, m_timeRange);
}

void SettingsViewModel::setAutoRefresh(bool value)
{
    if (m_autoRefresh == value) {
        return;
    }
    m_autoRefresh = value;
    emit autoRefreshChanged();
    saveSetting(kAutoRefreshKey, m_autoRefresh);
}

void SettingsViewModel::setDeviceCount(int value)
{
    value = qBound(1, value, 200);
    if (m_deviceCount == value) {
        return;
    }
    m_deviceCount = value;
    emit deviceCountChanged();
    saveSetting(kDeviceCountKey, m_deviceCount);
}

void SettingsViewModel::setFaultRate(double value)
{
    value = qBound(0.0, value, 1.0);
    if (qFuzzyCompare(m_faultRate, value)) {
        return;
    }
    m_faultRate = value;
    emit faultRateChanged();
    saveSetting(kFaultRateKey, m_faultRate);
}

void SettingsViewModel::setRefreshHz(int value)
{
    value = qBound(1, value, 10);
    if (m_refreshHz == value) {
        return;
    }
    m_refreshHz = value;
    emit refreshHzChanged();
    saveSetting(kRefreshHzKey, m_refreshHz);
}

void SettingsViewModel::setVibrationThreshold(double value)
{
    value = qBound(0.0, value, 1000.0);
    if (qFuzzyCompare(m_vibrationThreshold, value)) {
        return;
    }
    m_vibrationThreshold = value;
    emit vibrationThresholdChanged();
    saveSetting(kVibrationThresholdKey, m_vibrationThreshold);
}

void SettingsViewModel::setTorqueThreshold(double value)
{
    value = qBound(0.0, value, 1000.0);
    if (qFuzzyCompare(m_torqueThreshold, value)) {
        return;
    }
    m_torqueThreshold = value;
    emit torqueThresholdChanged();
    saveSetting(kTorqueThresholdKey, m_torqueThreshold);
}

void SettingsViewModel::setCurrentThreshold(double value)
{
    value = qBound(0.0, value, 1000.0);
    if (qFuzzyCompare(m_currentThreshold, value)) {
        return;
    }
    m_currentThreshold = value;
    emit currentThresholdChanged();
    saveSetting(kCurrentThresholdKey, m_currentThreshold);
}

void SettingsViewModel::loadSettings()
{
    QSettings settings;
    m_language = settings.value(kLanguageKey, m_language).toString();
    m_theme = normalizeTheme(settings.value(kThemeKey, m_theme).toString());
    m_timeRange = normalizeTimeRange(settings.value(kTimeRangeKey, m_timeRange).toString());
    m_autoRefresh = settings.value(kAutoRefreshKey, m_autoRefresh).toBool();
    m_deviceCount = qBound(1, settings.value(kDeviceCountKey, m_deviceCount).toInt(), 200);
    m_faultRate = qBound(0.0, settings.value(kFaultRateKey, m_faultRate).toDouble(), 1.0);
    m_refreshHz = qBound(1, settings.value(kRefreshHzKey, m_refreshHz).toInt(), 10);
    m_vibrationThreshold = qBound(0.0, settings.value(kVibrationThresholdKey, m_vibrationThreshold).toDouble(), 1000.0);
    m_torqueThreshold = qBound(0.0, settings.value(kTorqueThresholdKey, m_torqueThreshold).toDouble(), 1000.0);
    m_currentThreshold = qBound(0.0, settings.value(kCurrentThresholdKey, m_currentThreshold).toDouble(), 1000.0);
}

void SettingsViewModel::saveSetting(const QString &key, const QVariant &value)
{
    QSettings settings;
    settings.setValue(key, value);
}
