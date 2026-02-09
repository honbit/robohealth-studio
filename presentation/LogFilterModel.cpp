#include "LogFilterModel.h"

#include <QAbstractItemModel>
#include <QTime>

namespace {
constexpr int kLevelRole = Qt::UserRole + 1;
constexpr int kMessageRole = Qt::UserRole + 2;
constexpr int kDeviceRole = Qt::UserRole + 3;
constexpr int kTimeRole = Qt::UserRole + 4;
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

LogFilterModel::LogFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

QString LogFilterModel::searchText() const
{
    return m_searchText;
}

QString LogFilterModel::levelFilter() const
{
    return m_levelFilter;
}

QString LogFilterModel::timeRange() const
{
    return m_timeRange;
}

void LogFilterModel::setSearchText(const QString &value)
{
    if (m_searchText == value) {
        return;
    }
    m_searchText = value;
    emit searchTextChanged();
    beginFilterChange();
    endFilterChange();
}

void LogFilterModel::setLevelFilter(const QString &value)
{
    if (m_levelFilter == value) {
        return;
    }
    m_levelFilter = value;
    emit levelFilterChanged();
    beginFilterChange();
    endFilterChange();
}

void LogFilterModel::setTimeRange(const QString &value)
{
    if (m_timeRange == value) {
        return;
    }
    m_timeRange = value;
    emit timeRangeChanged();
    beginFilterChange();
    endFilterChange();
}

bool LogFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!sourceModel()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString level = sourceModel()->data(index, kLevelRole).toString();
    const QString message = sourceModel()->data(index, kMessageRole).toString();
    const QString device = sourceModel()->data(index, kDeviceRole).toString();
    const QString time = sourceModel()->data(index, kTimeRole).toString();

    if (!m_levelFilter.isEmpty() && level != m_levelFilter) {
        return false;
    }

    if (!m_searchText.trimmed().isEmpty()) {
        const QString keyword = m_searchText.trimmed();
        if (!message.contains(keyword, Qt::CaseInsensitive)
            && !device.contains(keyword, Qt::CaseInsensitive)
            && !level.contains(keyword, Qt::CaseInsensitive)) {
            return false;
        }
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

    return true;
}
