#pragma once

#include <QString>
#include "../models/DiagnosticCard.h"

class IDiagnosticsEngine
{
public:
    virtual ~IDiagnosticsEngine() = default;
    virtual DiagnosticCard diagnose(const QString &deviceId) = 0;
};
