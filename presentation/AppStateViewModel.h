#pragma once

#include "ViewModelBase.h"

class AppStateViewModel : public ViewModelBase
{
    Q_OBJECT
public:
    explicit AppStateViewModel(QObject *parent = nullptr) : ViewModelBase(parent) {}

    // TODO: add Q_PROPERTY for UI bindings
};
