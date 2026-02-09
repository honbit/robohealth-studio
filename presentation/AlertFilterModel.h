#pragma once

#include <QSortFilterProxyModel>

class AlertFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString levelFilter READ levelFilter WRITE setLevelFilter NOTIFY levelFilterChanged)
    Q_PROPERTY(QString deviceFilter READ deviceFilter WRITE setDeviceFilter NOTIFY deviceFilterChanged)
    Q_PROPERTY(QString timeRange READ timeRange WRITE setTimeRange NOTIFY timeRangeChanged)
    Q_PROPERTY(QString statusFilter READ statusFilter WRITE setStatusFilter NOTIFY statusFilterChanged)

public:
    explicit AlertFilterModel(QObject *parent = nullptr);

    QString levelFilter() const;
    QString deviceFilter() const;
    QString timeRange() const;
    QString statusFilter() const;

    void setLevelFilter(const QString &value);
    void setDeviceFilter(const QString &value);
    void setTimeRange(const QString &value);
    void setStatusFilter(const QString &value);

signals:
    void levelFilterChanged();
    void deviceFilterChanged();
    void timeRangeChanged();
    void statusFilterChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_levelFilter;
    QString m_deviceFilter;
    QString m_timeRange;
    QString m_statusFilter;
};
