#pragma once

#include <QString>
#include <QDateTime>

struct Alert
{
    QString id;
    QString deviceId;
    QString level; // info/warn/critical
    QString message;
    QDateTime timestamp;
};
