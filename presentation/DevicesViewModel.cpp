#include "DevicesViewModel.h"

#include "TelemetryServiceBase.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QTime>

DeviceListModel::DeviceListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_items = {
        {"Robot-01", "Robot", "Online", "10:46", 92},
        {"Robot-02", "Robot", "Online", "10:45", 88},
        {"Cam-04", "Camera", "Offline", "09:50", 61},
        {"IMU-01", "IMU", "Online", "10:44", 95},
        {"PLC-01", "PLC", "Online", "10:40", 90}
    };
}

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.size();
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return {};
    }
    const DeviceItem &item = m_items.at(index.row());
    switch (role) {
    case NameRole: return item.name;
    case TypeRole: return item.type;
    case StatusRole: return item.status;
    case LastSeenRole: return item.lastSeen;
    case HealthRole: return item.health;
    default: return {};
    }
}

QHash<int, QByteArray> DeviceListModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {TypeRole, "type"},
        {StatusRole, "status"},
        {LastSeenRole, "lastSeen"},
        {HealthRole, "health"}
    };
}

const DeviceItem *DeviceListModel::itemAt(int row) const
{
    if (row < 0 || row >= m_items.size()) {
        return nullptr;
    }
    return &m_items[row];
}

void DeviceListModel::setItems(const QVector<DeviceItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

void DeviceListModel::updateHealth(int row, int value)
{
    if (row < 0 || row >= m_items.size()) {
        return;
    }
    m_items[row].health = value;
    const QModelIndex index = createIndex(row, 0);
    emit dataChanged(index, index, {HealthRole});
}

void DeviceListModel::updateLastSeen(int row, const QString &value)
{
    if (row < 0 || row >= m_items.size()) {
        return;
    }
    m_items[row].lastSeen = value;
    const QModelIndex index = createIndex(row, 0);
    emit dataChanged(index, index, {LastSeenRole});
}

DevicesViewModel::DevicesViewModel(TelemetryServiceBase *service, QObject *parent)
    : ViewModelBase(parent)
    , m_service(service)
{
    m_filteredDevices.setSourceModel(&m_devices);

    connect(&m_filteredDevices, &QAbstractItemModel::modelReset, this, &DevicesViewModel::ensureSelectionValid);
    connect(&m_filteredDevices, &QAbstractItemModel::rowsInserted, this, &DevicesViewModel::ensureSelectionValid);
    connect(&m_filteredDevices, &QAbstractItemModel::rowsRemoved, this, &DevicesViewModel::ensureSelectionValid);
    connect(&m_filteredDevices, &QAbstractItemModel::layoutChanged, this, &DevicesViewModel::ensureSelectionValid);

    connect(&m_devices, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
                handleDevicesChanged(topLeft.row(), bottomRight.row());
            });

    if (m_service) {
        refreshDevices();
        connect(m_service, &TelemetryServiceBase::devicesUpdated, this, &DevicesViewModel::refreshDevices);
    } else {
        m_timer.setInterval(1500);
        connect(&m_timer, &QTimer::timeout, this, &DevicesViewModel::tick);
        m_timer.start();
    }

    updateDeviceTrend();
}

DeviceListModel *DevicesViewModel::devices()
{
    return &m_devices;
}

DeviceFilterModel *DevicesViewModel::filteredDevices()
{
    return &m_filteredDevices;
}

int DevicesViewModel::selectedIndex() const
{
    return m_selectedIndex;
}

void DevicesViewModel::setSelectedIndex(int index)
{
    if (m_selectedIndex == index) {
        return;
    }
    m_selectedIndex = index;
    emit selectedIndexChanged();
    emit selectedDeviceChanged();
    updateDeviceTrend();
}

QVariantMap DevicesViewModel::selectedDevice() const
{
    QVariantMap map;
    const QModelIndex proxyIndex = m_filteredDevices.index(m_selectedIndex, 0);
    if (!proxyIndex.isValid()) {
        return map;
    }
    const QModelIndex sourceIndex = m_filteredDevices.mapToSource(proxyIndex);
    const DeviceItem *item = m_devices.itemAt(sourceIndex.row());
    if (!item) {
        return map;
    }
    map.insert("name", item->name);
    map.insert("type", item->type);
    map.insert("status", item->status);
    map.insert("lastSeen", item->lastSeen);
    map.insert("health", item->health);
    return map;
}

QVariantList DevicesViewModel::deviceTrendPoints() const
{
    return m_deviceTrendPoints;
}

void DevicesViewModel::connectAll()
{
    // TODO: 接口未开发，当前使用 TelemetryServiceBase。
    if (!m_service) {
        qInfo() << "connectAll requested, but no telemetry service attached.";
        return;
    }
    m_service->connectAll();
}

void DevicesViewModel::disconnectSelected()
{
    // TODO: 接口未开发，当前使用 TelemetryServiceBase。
    if (!m_service) {
        qInfo() << "disconnectSelected requested, but no telemetry service attached.";
        return;
    }
    const QString name = selectedDevice().value("name").toString();
    m_service->disconnectDevice(name);
}

void DevicesViewModel::simulateFault()
{
    // TODO: 接口未开发，当前使用 TelemetryServiceBase。
    if (!m_service) {
        qInfo() << "simulateFault requested, but no telemetry service attached.";
        return;
    }
    const QString name = selectedDevice().value("name").toString();
    m_service->simulateFault(name);
}

void DevicesViewModel::tick()
{
    if (m_service) {
        return;
    }
    if (m_devices.rowCount() == 0) {
        return;
    }
    int row = QRandomGenerator::global()->bounded(m_devices.rowCount());
    const DeviceItem *item = m_devices.itemAt(row);
    if (!item) {
        return;
    }
    int delta = QRandomGenerator::global()->bounded(-3, 4);
    int nextHealth = qBound(50, item->health + delta, 100);
    m_devices.updateHealth(row, nextHealth);
    m_devices.updateLastSeen(row, QTime::currentTime().toString("HH:mm"));
    updateDeviceTrend();
}

void DevicesViewModel::refreshDevices()
{
    if (!m_service) {
        return;
    }
    m_devices.setItems(m_service->devices());
    updateDeviceTrend();
}

void DevicesViewModel::handleDevicesChanged(int topRow, int bottomRow)
{
    if (m_selectedIndex < 0) {
        return;
    }
    const QModelIndex proxyIndex = m_filteredDevices.index(m_selectedIndex, 0);
    if (!proxyIndex.isValid()) {
        return;
    }
    const int sourceRow = m_filteredDevices.mapToSource(proxyIndex).row();
    if (sourceRow >= topRow && sourceRow <= bottomRow) {
        emit selectedDeviceChanged();
        updateDeviceTrend();
    }
}

void DevicesViewModel::ensureSelectionValid()
{
    const int count = m_filteredDevices.rowCount();
    int nextIndex = m_selectedIndex;
    if (count == 0) {
        nextIndex = -1;
    } else if (m_selectedIndex < 0 || m_selectedIndex >= count) {
        nextIndex = 0;
    }

    if (nextIndex != m_selectedIndex) {
        m_selectedIndex = nextIndex;
        emit selectedIndexChanged();
    }
    emit selectedDeviceChanged();
    updateDeviceTrend();
}

void DevicesViewModel::updateDeviceTrend()
{
    m_deviceTrendPoints.clear();
    const QVariantMap device = selectedDevice();
    if (device.isEmpty()) {
        emit deviceTrendPointsChanged();
        return;
    }

    const int health = device.value("health").toInt();
    double base = qBound(0.2, health / 100.0, 0.95);
    if (device.value("status").toString() == "Offline") {
        base = qBound(0.1, base - 0.2, 0.9);
    }

    m_deviceTrendPoints.reserve(16);
    for (int i = 0; i < 16; ++i) {
        const double jitter = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.25;
        const double value = qBound(0.05, base + jitter, 0.98);
        m_deviceTrendPoints.append(value);
    }
    emit deviceTrendPointsChanged();
}
