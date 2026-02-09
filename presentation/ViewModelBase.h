#pragma once

#include <QObject>

class ViewModelBase : public QObject
{
public:
    explicit ViewModelBase(QObject *parent = nullptr) : QObject(parent) {}
};
