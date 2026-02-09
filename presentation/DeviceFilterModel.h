#pragma once

#include <QSortFilterProxyModel>

class DeviceFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(QString typeFilter READ typeFilter WRITE setTypeFilter NOTIFY typeFilterChanged)
    Q_PROPERTY(QString statusFilter READ statusFilter WRITE setStatusFilter NOTIFY statusFilterChanged)

public:
    explicit DeviceFilterModel(QObject *parent = nullptr);

    QString searchText() const;
    QString typeFilter() const;
    QString statusFilter() const;

    void setSearchText(const QString &value);
    void setTypeFilter(const QString &value);
    void setStatusFilter(const QString &value);

signals:
    void searchTextChanged();
    void typeFilterChanged();
    void statusFilterChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_searchText;
    QString m_typeFilter;
    QString m_statusFilter;
};
