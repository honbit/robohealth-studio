#pragma once

#include <QList>
#include "../models/Device.h"

class IDeviceGateway
{
public:
    virtual ~IDeviceGateway() = default;
    virtual QList<Device> listDevices() = 0;
    virtual void connectAll() = 0;
    virtual void disconnectAll() = 0;
};
