#pragma once

#include "ViewModelBase.h"

#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QVariantList>
#include <QVector>

struct ReplayEventItem {
    QString time;
    QString text;
};

class ReplayEventModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        TimeRole = Qt::UserRole + 1,
        TextRole
    };

    explicit ReplayEventModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setEvents(const QVector<ReplayEventItem> &events);

private:
    QVector<ReplayEventItem> m_events;
};

class ReplayViewModel : public ViewModelBase
{
    Q_OBJECT
    Q_PROPERTY(ReplayEventModel* events READ events CONSTANT)
    Q_PROPERTY(QVariantList trendPoints READ trendPoints NOTIFY trendPointsChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString timeLabel READ timeLabel NOTIFY timeLabelChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(QString timeRange READ timeRange WRITE setTimeRange NOTIFY timeRangeChanged)
    Q_PROPERTY(QString session READ session WRITE setSession NOTIFY sessionChanged)
    Q_PROPERTY(QString device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(double playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY playbackRateChanged)

public:
    explicit ReplayViewModel(const QUrl &apiBase = QUrl(), bool useRest = false, QObject *parent = nullptr);

    ReplayEventModel *events();
    QVariantList trendPoints() const;
    double progress() const;
    QString timeLabel() const;
    bool playing() const;
    QString timeRange() const;
    void setTimeRange(const QString &value);
    QString session() const;
    void setSession(const QString &value);
    QString device() const;
    void setDevice(const QString &value);
    double playbackRate() const;
    void setPlaybackRate(double rate);

    Q_INVOKABLE void loadSession();
    Q_INVOKABLE void start();
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();

signals:
    void trendPointsChanged();
    void progressChanged();
    void timeLabelChanged();
    void playingChanged();
    void timeRangeChanged();
    void sessionChanged();
    void deviceChanged();
    void playbackRateChanged();

private:
    void tick();
    void resetTimeline();
    void fetchReplay();
    void updateTrend();
    QString formatTime(int seconds) const;
    void applyPlaybackRate();

    ReplayEventModel m_events;
    QTimer m_timer;
    QVariantList m_trendPoints;
    int m_durationSeconds = 300;
    int m_currentSeconds = 0;
    bool m_playing = false;
    int m_maxPoints = 20;
    QString m_timeRange = "24h";
    QString m_session = "Session A";
    QString m_device = "Robot-01";
    double m_playbackRate = 1.0;
    bool m_useRest = false;
    QUrl m_apiBase;
    QNetworkAccessManager m_network;
    int m_requestId = 0;
    bool m_fetchInFlight = false;
};
