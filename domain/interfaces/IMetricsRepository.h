#pragma once

#include <QList>
#include <QString>
#include "../models/MetricSample.h"

class IMetricsRepository
{
public:
    virtual ~IMetricsRepository() = default;
    virtual QList<MetricSample> latestSamples(const QString &deviceId) = 0;
};
