#pragma once

#include <QString>

class IReplayStore
{
public:
    virtual ~IReplayStore() = default;
    virtual bool startRecording(const QString &sessionId) = 0;
    virtual void stopRecording() = 0;
    virtual bool loadSession(const QString &sessionId) = 0;
};
