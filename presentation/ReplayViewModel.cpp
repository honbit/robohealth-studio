#include "ReplayViewModel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRandomGenerator>
#include <QScopedPointer>
#include <QUrlQuery>
#include <QtGlobal>

namespace {
QUrl resolveUrl(const QUrl &baseUrl, const QString &path)
{
    QUrl resolved = baseUrl;
    if (!resolved.path().endsWith('/')) {
        resolved.setPath(resolved.path() + '/');
    }
    return resolved.resolved(QUrl(path.startsWith('/') ? path.mid(1) : path));
}
}

ReplayEventModel::ReplayEventModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ReplayEventModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_events.size();
}

QVariant ReplayEventModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_events.size()) {
        return {};
    }

    const ReplayEventItem &item = m_events.at(index.row());
    switch (role) {
    case TimeRole: return item.time;
    case TextRole: return item.text;
    default: return {};
    }
}

QHash<int, QByteArray> ReplayEventModel::roleNames() const
{
    return {
        {TimeRole, "time"},
        {TextRole, "text"}
    };
}

void ReplayEventModel::setEvents(const QVector<ReplayEventItem> &events)
{
    beginResetModel();
    m_events = events;
    endResetModel();
}

ReplayViewModel::ReplayViewModel(const QUrl &apiBase, bool useRest, QObject *parent)
    : ViewModelBase(parent)
    , m_useRest(useRest)
    , m_apiBase(apiBase)
{
    if (m_useRest && !m_apiBase.isValid()) {
        m_apiBase = QUrl(QStringLiteral("http://127.0.0.1:5000"));
    }
    connect(&m_timer, &QTimer::timeout, this, &ReplayViewModel::tick);
    applyPlaybackRate();

    resetTimeline();
}

ReplayEventModel *ReplayViewModel::events()
{
    return &m_events;
}

QVariantList ReplayViewModel::trendPoints() const
{
    return m_trendPoints;
}

double ReplayViewModel::progress() const
{
    if (m_durationSeconds <= 0) {
        return 0.0;
    }
    return static_cast<double>(m_currentSeconds) / static_cast<double>(m_durationSeconds);
}

QString ReplayViewModel::timeLabel() const
{
    return formatTime(m_currentSeconds) + " / " + formatTime(m_durationSeconds);
}

bool ReplayViewModel::playing() const
{
    return m_playing;
}

QString ReplayViewModel::timeRange() const
{
    return m_timeRange;
}

QString ReplayViewModel::session() const
{
    return m_session;
}

QString ReplayViewModel::device() const
{
    return m_device;
}

void ReplayViewModel::setTimeRange(const QString &value)
{
    QString normalized = value;
    if (normalized != "1h" && normalized != "24h" && normalized != "7d") {
        normalized = "24h";
    }
    if (m_timeRange == normalized) {
        return;
    }
    m_timeRange = normalized;
    emit timeRangeChanged();
    if (m_useRest && m_apiBase.isValid()) {
        fetchReplay();
    } else {
        resetTimeline();
    }
}

void ReplayViewModel::setSession(const QString &value)
{
    if (m_session == value) {
        return;
    }
    m_session = value;
    emit sessionChanged();
    if (m_useRest && m_apiBase.isValid()) {
        fetchReplay();
    } else {
        resetTimeline();
    }
}

void ReplayViewModel::setDevice(const QString &value)
{
    if (m_device == value) {
        return;
    }
    m_device = value;
    emit deviceChanged();
    if (m_useRest && m_apiBase.isValid()) {
        fetchReplay();
    } else {
        resetTimeline();
    }
}

double ReplayViewModel::playbackRate() const
{
    return m_playbackRate;
}

void ReplayViewModel::setPlaybackRate(double rate)
{
    rate = qBound(0.25, rate, 8.0);
    if (qFuzzyCompare(m_playbackRate, rate)) {
        return;
    }
    m_playbackRate = rate;
    applyPlaybackRate();
    emit playbackRateChanged();
}

void ReplayViewModel::loadSession()
{
    if (m_useRest && m_apiBase.isValid()) {
        if (!m_fetchInFlight) {
            fetchReplay();
        }
        return;
    }
    resetTimeline();
}

void ReplayViewModel::start()
{
    m_currentSeconds = 0;
    emit progressChanged();
    emit timeLabelChanged();
    play();
}

void ReplayViewModel::play()
{
    if (m_playing) {
        return;
    }
    m_playing = true;
    emit playingChanged();
    applyPlaybackRate();
    m_timer.start();
}

void ReplayViewModel::pause()
{
    if (!m_playing) {
        return;
    }
    m_playing = false;
    emit playingChanged();
    m_timer.stop();
}

void ReplayViewModel::stop()
{
    if (m_playing) {
        m_playing = false;
        emit playingChanged();
    }
    m_timer.stop();
    m_currentSeconds = 0;
    emit progressChanged();
    emit timeLabelChanged();
}

void ReplayViewModel::tick()
{
    if (!m_playing) {
        return;
    }

    if (m_currentSeconds >= m_durationSeconds) {
        pause();
        return;
    }

    m_currentSeconds += 1;
    emit progressChanged();
    emit timeLabelChanged();
    updateTrend();
}

void ReplayViewModel::resetTimeline()
{
    m_currentSeconds = 0;
    m_playing = false;
    m_timer.stop();
    emit playingChanged();
    emit progressChanged();
    emit timeLabelChanged();

    const uint seed = qHash(m_session + "|" + m_device + "|" + m_timeRange);
    QRandomGenerator generator(seed);
    QVector<ReplayEventItem> events;
    if (m_timeRange == "1h") {
        m_durationSeconds = 300;
        m_maxPoints = 20;
        events = {
            {"00:10", "Replay started"},
            {"01:22", "Warning spike"},
            {"03:45", "Recovered"}
        };
    } else if (m_timeRange == "7d") {
        m_durationSeconds = 1800;
        m_maxPoints = 40;
        events = {
            {"01:30", "Daily load"},
            {"08:10", "Warning spike"},
            {"12:20", "Critical stop"},
            {"18:05", "Recovered"}
        };
    } else {
        m_durationSeconds = 900;
        m_maxPoints = 30;
        events = {
            {"00:30", "Replay started"},
            {"04:10", "Warning spike"},
            {"08:45", "Recovered"}
        };
    }

    const int offsetSeconds = static_cast<int>(generator.bounded(0, 45));
    for (auto &event : events) {
        const QStringList parts = event.time.split(":");
        if (parts.size() == 2) {
            bool okMin = false;
            bool okSec = false;
            int minutes = parts.at(0).toInt(&okMin);
            int seconds = parts.at(1).toInt(&okSec);
            if (okMin && okSec) {
                int total = minutes * 60 + seconds + offsetSeconds;
                const int max = qMax(1, m_durationSeconds);
                total = total % max;
                const int adjMin = total / 60;
                const int adjSec = total % 60;
                event.time = QStringLiteral("%1:%2")
                    .arg(adjMin, 2, 10, QLatin1Char('0'))
                    .arg(adjSec, 2, 10, QLatin1Char('0'));
            }
        }
    }
    m_events.setEvents(events);

    m_trendPoints.clear();
    m_trendPoints.reserve(m_maxPoints);
    const double base = 0.15 + generator.generateDouble() * 0.2;
    const double step = m_maxPoints > 1 ? 0.6 / (m_maxPoints - 1) : 0.0;
    for (int i = 0; i < m_maxPoints; ++i) {
        m_trendPoints.append(qBound(0.05, base + step * i, 0.95));
    }
    emit trendPointsChanged();
}

void ReplayViewModel::fetchReplay()
{
    if (!m_apiBase.isValid()) {
        resetTimeline();
        return;
    }
    m_fetchInFlight = true;
    const int requestId = ++m_requestId;
    QUrl url = resolveUrl(m_apiBase, "/api/replay");
    QUrlQuery query;
    query.addQueryItem("session", m_session);
    query.addQueryItem("device", m_device);
    query.addQueryItem("timeRange", m_timeRange);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, requestId]() {
        const QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
        m_fetchInFlight = false;
        if (requestId != m_requestId) {
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            resetTimeline();
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isObject()) {
            resetTimeline();
            return;
        }
        const QJsonObject payload = doc.object();
        m_durationSeconds = payload.value("durationSeconds").toInt(m_durationSeconds);
        m_currentSeconds = 0;
        if (m_playing) {
            m_playing = false;
            emit playingChanged();
        }
        m_timer.stop();
        emit progressChanged();
        emit timeLabelChanged();

        const QJsonArray events = payload.value("events").toArray();
        QVector<ReplayEventItem> items;
        items.reserve(events.size());
        for (const QJsonValue &value : events) {
            const QJsonObject item = value.toObject();
            ReplayEventItem event;
            event.time = item.value("time").toString();
            event.text = item.value("text").toString();
            items.push_back(event);
        }
        m_events.setEvents(items);

        m_trendPoints.clear();
        const QJsonArray trend = payload.value("trendPoints").toArray();
        m_trendPoints.reserve(trend.size());
        for (const QJsonValue &value : trend) {
            m_trendPoints.append(value.toDouble());
        }
        if (!trend.isEmpty()) {
            m_maxPoints = trend.size();
        }
        emit trendPointsChanged();
    });
}

void ReplayViewModel::updateTrend()
{
    if (m_trendPoints.size() >= m_maxPoints) {
        m_trendPoints.removeFirst();
    }
    const double value = 0.2 + QRandomGenerator::global()->generateDouble() * 0.7;
    m_trendPoints.append(value);
    emit trendPointsChanged();
}

QString ReplayViewModel::formatTime(int seconds) const
{
    const int mins = seconds / 60;
    const int secs = seconds % 60;
    return QStringLiteral("%1:%2")
        .arg(mins, 2, 10, QLatin1Char('0'))
        .arg(secs, 2, 10, QLatin1Char('0'));
}

void ReplayViewModel::applyPlaybackRate()
{
    const int interval = qMax(50, static_cast<int>(1000.0 / m_playbackRate));
    m_timer.setInterval(interval);
}
