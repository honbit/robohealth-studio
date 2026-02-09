#pragma once

#include "DeviceFilterModel.h"
#include "TelemetryItems.h"
#include "ViewModelBase.h"

#include <QAbstractListModel>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

class TelemetryServiceBase;

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        TypeRole,
        StatusRole,
        LastSeenRole,
        HealthRole
    };

    explicit DeviceListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    const DeviceItem *itemAt(int row) const;
    void setItems(const QVector<DeviceItem> &items);
    void updateHealth(int row, int value);
    void updateLastSeen(int row, const QString &value);

private:
    QVector<DeviceItem> m_items;
};

class DevicesViewModel : public ViewModelBase
{
    Q_OBJECT
    Q_PROPERTY(DeviceListModel* devices READ devices CONSTANT)
    Q_PROPERTY(DeviceFilterModel* filteredDevices READ filteredDevices CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(QVariantMap selectedDevice READ selectedDevice NOTIFY selectedDeviceChanged)
    Q_PROPERTY(QVariantList deviceTrendPoints READ deviceTrendPoints NOTIFY deviceTrendPointsChanged)

public:
    explicit DevicesViewModel(TelemetryServiceBase *service = nullptr, QObject *parent = nullptr);

    DeviceListModel *devices();
    DeviceFilterModel *filteredDevices();
    int selectedIndex() const;
    void setSelectedIndex(int index);
    QVariantMap selectedDevice() const;
    QVariantList deviceTrendPoints() const;

    Q_INVOKABLE void connectAll();
    Q_INVOKABLE void disconnectSelected();
    Q_INVOKABLE void simulateFault();

signals:
    void selectedIndexChanged();
    void selectedDeviceChanged();
    void deviceTrendPointsChanged();

private:
    void tick();
    void refreshDevices();
    void handleDevicesChanged(int topRow, int bottomRow);
    void ensureSelectionValid();
    void updateDeviceTrend();

    DeviceListModel m_devices;
    DeviceFilterModel m_filteredDevices;
    int m_selectedIndex = 0;
    QTimer m_timer;
    TelemetryServiceBase *m_service = nullptr;
    QVariantList m_deviceTrendPoints;
};
