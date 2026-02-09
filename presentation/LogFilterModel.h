#pragma once

#include <QSortFilterProxyModel>

class LogFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(QString levelFilter READ levelFilter WRITE setLevelFilter NOTIFY levelFilterChanged)
    Q_PROPERTY(QString timeRange READ timeRange WRITE setTimeRange NOTIFY timeRangeChanged)

public:
    explicit LogFilterModel(QObject *parent = nullptr);

    QString searchText() const;
    QString levelFilter() const;
    QString timeRange() const;

    void setSearchText(const QString &value);
    void setLevelFilter(const QString &value);
    void setTimeRange(const QString &value);

signals:
    void searchTextChanged();
    void levelFilterChanged();
    void timeRangeChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_searchText;
    QString m_levelFilter;
    QString m_timeRange;
};
