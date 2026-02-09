#pragma once

#include <QString>

struct Device
{
    QString id;
    QString name;
    QString type;
    bool online = false;
};
