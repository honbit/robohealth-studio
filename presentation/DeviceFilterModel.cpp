#include "DeviceFilterModel.h"

#include <QAbstractItemModel>

namespace {
constexpr int kNameRole = Qt::UserRole + 1;
constexpr int kTypeRole = Qt::UserRole + 2;
constexpr int kStatusRole = Qt::UserRole + 3;
}

DeviceFilterModel::DeviceFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

QString DeviceFilterModel::searchText() const
{
    return m_searchText;
}

QString DeviceFilterModel::typeFilter() const
{
    return m_typeFilter;
}

QString DeviceFilterModel::statusFilter() const
{
    return m_statusFilter;
}

void DeviceFilterModel::setSearchText(const QString &value)
{
    if (m_searchText == value) {
        return;
    }
    m_searchText = value;
    emit searchTextChanged();
    beginFilterChange();
    endFilterChange();
}

void DeviceFilterModel::setTypeFilter(const QString &value)
{
    if (m_typeFilter == value) {
        return;
    }
    m_typeFilter = value;
    emit typeFilterChanged();
    beginFilterChange();
    endFilterChange();
}

void DeviceFilterModel::setStatusFilter(const QString &value)
{
    if (m_statusFilter == value) {
        return;
    }
    m_statusFilter = value;
    emit statusFilterChanged();
    beginFilterChange();
    endFilterChange();
}

bool DeviceFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!sourceModel()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString name = sourceModel()->data(index, kNameRole).toString();
    const QString type = sourceModel()->data(index, kTypeRole).toString();
    const QString status = sourceModel()->data(index, kStatusRole).toString();

    if (!m_searchText.trimmed().isEmpty()) {
        const QString keyword = m_searchText.trimmed();
        if (!name.contains(keyword, Qt::CaseInsensitive)
            && !type.contains(keyword, Qt::CaseInsensitive)
            && !status.contains(keyword, Qt::CaseInsensitive)) {
            return false;
        }
    }

    if (!m_typeFilter.isEmpty() && type != m_typeFilter) {
        return false;
    }

    if (!m_statusFilter.isEmpty() && status != m_statusFilter) {
        return false;
    }

    return true;
}
