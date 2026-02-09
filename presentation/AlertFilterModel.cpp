#include "AlertFilterModel.h"

#include <QAbstractItemModel>
#include <QTime>

namespace {
constexpr int kLevelRole = Qt::UserRole + 1;
constexpr int kDeviceRole = Qt::UserRole + 3;
constexpr int kTimeRole = Qt::UserRole + 4;
constexpr int kAcknowledgedRole = Qt::UserRole + 7;
constexpr int kMutedRole = Qt::UserRole + 8;
constexpr int kMinutesPerDay = 24 * 60;

int minutesAgo(const QString &timeText)
{
    if (timeText.isEmpty()) {
        return -1;
    }
    QTime parsed = QTime::fromString(timeText, "HH:mm");
    if (!parsed.isValid()) {
        parsed = QTime::fromString(timeText, "HH:mm:ss");
    }
    if (!parsed.isValid()) {
        return -1;
    }
    const QTime now = QTime::currentTime();
    int diffMinutes = parsed.secsTo(now) / 60;
    if (diffMinutes < 0) {
        diffMinutes += kMinutesPerDay;
    }
    return diffMinutes;
}
}

AlertFilterModel::AlertFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

QString AlertFilterModel::levelFilter() const
{
    return m_levelFilter;
}

QString AlertFilterModel::deviceFilter() const
{
    return m_deviceFilter;
}

QString AlertFilterModel::timeRange() const
{
    return m_timeRange;
}

QString AlertFilterModel::statusFilter() const
{
    return m_statusFilter;
}

void AlertFilterModel::setLevelFilter(const QString &value)
{
    if (m_levelFilter == value) {
        return;
    }
    m_levelFilter = value;
    emit levelFilterChanged();
    beginFilterChange();
    endFilterChange();
}

void AlertFilterModel::setDeviceFilter(const QString &value)
{
    if (m_deviceFilter == value) {
        return;
    }
    m_deviceFilter = value;
    emit deviceFilterChanged();
    beginFilterChange();
    endFilterChange();
}

void AlertFilterModel::setTimeRange(const QString &value)
{
    if (m_timeRange == value) {
        return;
    }
    m_timeRange = value;
    emit timeRangeChanged();
    beginFilterChange();
    endFilterChange();
}

void AlertFilterModel::setStatusFilter(const QString &value)
{
    if (m_statusFilter == value) {
        return;
    }
    m_statusFilter = value;
    emit statusFilterChanged();
    beginFilterChange();
    endFilterChange();
}

bool AlertFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!sourceModel()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString level = sourceModel()->data(index, kLevelRole).toString();
    const QString device = sourceModel()->data(index, kDeviceRole).toString();
    const QString time = sourceModel()->data(index, kTimeRole).toString();
    const bool acknowledged = sourceModel()->data(index, kAcknowledgedRole).toBool();
    const bool muted = sourceModel()->data(index, kMutedRole).toBool();

    if (!m_levelFilter.isEmpty() && level != m_levelFilter) {
        return false;
    }

    if (!m_deviceFilter.isEmpty() && device != m_deviceFilter) {
        return false;
    }

    if (!m_timeRange.isEmpty()) {
        const int minutes = minutesAgo(time);
        if (minutes >= 0) {
            int limitMinutes = 0;
            if (m_timeRange == "1h") {
                limitMinutes = 60;
            } else if (m_timeRange == "24h") {
                limitMinutes = 1440;
            } else if (m_timeRange == "7d") {
                limitMinutes = 10080;
            }
            if (limitMinutes > 0 && minutes > limitMinutes) {
                return false;
            }
        }
    }

    if (!m_statusFilter.isEmpty()) {
        if (m_statusFilter == "active") {
            if (acknowledged || muted) {
                return false;
            }
        } else if (m_statusFilter == "acknowledged") {
            if (!acknowledged) {
                return false;
            }
        } else if (m_statusFilter == "muted") {
            if (!muted) {
                return false;
            }
        }
    }

    return true;
}
