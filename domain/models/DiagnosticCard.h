#pragma once

#include <QString>

struct DiagnosticCard
{
    QString symptom;
    QString cause;
    QString evidence;
    QString action;
    double confidence = 0.0;
};
