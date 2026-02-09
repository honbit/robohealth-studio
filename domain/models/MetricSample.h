#pragma once

#include <QString>
#include <QDateTime>

struct MetricSample
{
    QString deviceId;
    QString metric;
    double value = 0.0;
    QDateTime timestamp;
};
