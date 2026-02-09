#pragma once

#include <QString>

struct DeviceItem {
    QString name;
    QString type;
    QString status;
    QString lastSeen;
    int health = 0;
};

struct AlertItem {
    QString id;
    QString level;
    QString message;
    QString device;
    QString time;
    QString evidence;
    QString action;
    bool acknowledged = false;
    bool muted = false;
};

struct LogItem {
    QString level;
    QString message;
    QString device;
    QString time;
};
