#pragma once

#include <QList>
#include "../models/Alert.h"

class IAlertRepository
{
public:
    virtual ~IAlertRepository() = default;
    virtual QList<Alert> recentAlerts() = 0;
};
