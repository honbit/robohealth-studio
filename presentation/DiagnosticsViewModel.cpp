#include "DiagnosticsViewModel.h"

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

DiagnosticCardModel::DiagnosticCardModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int DiagnosticCardModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_cards.size();
}

QVariant DiagnosticCardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_cards.size()) {
        return {};
    }

    const DiagnosticCardItem &card = m_cards.at(index.row());
    switch (role) {
    case SymptomRole: return card.symptom;
    case CauseRole: return card.cause;
    case EvidenceRole: return card.evidence;
    case ActionRole: return card.action;
    case ConfidenceRole: return card.confidence;
    default: return {};
    }
}

QHash<int, QByteArray> DiagnosticCardModel::roleNames() const
{
    return {
        {SymptomRole, "symptom"},
        {CauseRole, "cause"},
        {EvidenceRole, "evidence"},
        {ActionRole, "action"},
        {ConfidenceRole, "confidence"}
    };
}

void DiagnosticCardModel::setCards(const QVector<DiagnosticCardItem> &cards)
{
    beginResetModel();
    m_cards = cards;
    endResetModel();
}

const DiagnosticCardItem *DiagnosticCardModel::itemAt(int row) const
{
    if (row < 0 || row >= m_cards.size()) {
        return nullptr;
    }
    return &m_cards[row];
}

DiagnosticsViewModel::DiagnosticsViewModel(const QUrl &apiBase, bool useRest, QObject *parent)
    : ViewModelBase(parent)
    , m_useRest(useRest)
    , m_apiBase(apiBase)
{
    if (m_useRest && !m_apiBase.isValid()) {
        m_apiBase = QUrl(QStringLiteral("http://127.0.0.1:5000"));
    }
    m_checklist = {
        "Stop robot",
        "Inspect joint",
        "Apply lubricant",
        "Re-run calibration"
    };

    generate();
}

DiagnosticCardModel *DiagnosticsViewModel::cards()
{
    return &m_cards;
}

QVariantList DiagnosticsViewModel::evidencePoints() const
{
    return m_evidencePoints;
}

QStringList DiagnosticsViewModel::checklist() const
{
    return m_checklist;
}

QString DiagnosticsViewModel::selectedDevice() const
{
    return m_selectedDevice;
}

QString DiagnosticsViewModel::timeRange() const
{
    return m_timeRange;
}

void DiagnosticsViewModel::setSelectedDevice(const QString &value)
{
    if (m_selectedDevice == value) {
        return;
    }
    m_selectedDevice = value;
    emit selectedDeviceChanged();
    generate();
}

void DiagnosticsViewModel::setTimeRange(const QString &value)
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
    generate();
}

void DiagnosticsViewModel::generate()
{
    if (m_useRest && m_apiBase.isValid()) {
        fetchDiagnostics();
        return;
    }
    m_cards.setCards(buildCards());
    buildEvidence();
}

QVector<DiagnosticCardItem> DiagnosticsViewModel::buildCards() const
{
    struct Template {
        const char *symptom;
        const char *cause;
        const char *evidence;
        const char *action;
    };

    const Template templates[] = {
        {"Vibration spike", "Loose bearing", "Vibration > 0.4g", "Stop and inspect"},
        {"Torque drift", "Calibration offset", "Torque > 20N*m", "Re-calibrate"},
        {"Current surge", "Cable wear", "Current > 15A", "Check wiring"},
        {"Temperature rise", "Cooling reduced", "Temp > 55C", "Inspect fan"}
    };

    QVector<DiagnosticCardItem> cards;
    cards.reserve(2);

    const int first = QRandomGenerator::global()->bounded(0, 2);
    const int second = QRandomGenerator::global()->bounded(2, 4);
    double confidenceBoost = 0.0;
    if (m_timeRange == "1h") {
        confidenceBoost = 0.05;
    } else if (m_timeRange == "7d") {
        confidenceBoost = -0.05;
    }

    for (int idx : {first, second}) {
        const auto &entry = templates[idx];
        DiagnosticCardItem card;
        card.symptom = entry.symptom;
        card.cause = entry.cause;
        card.evidence = entry.evidence;
        card.action = entry.action;
        card.confidence = 0.55 + QRandomGenerator::global()->generateDouble() * 0.4 + confidenceBoost;
        card.confidence = qBound(0.4, card.confidence, 0.95);
        cards.push_back(card);
    }

    return cards;
}

void DiagnosticsViewModel::fetchDiagnostics()
{
    const int requestId = ++m_requestId;
    QUrl url = resolveUrl(m_apiBase, "/api/diagnostics");
    QUrlQuery query;
    query.addQueryItem("device", m_selectedDevice);
    query.addQueryItem("timeRange", m_timeRange);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, requestId]() {
        const QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
        if (requestId != m_requestId) {
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            m_cards.setCards(buildCards());
            buildEvidence();
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isObject()) {
            m_cards.setCards(buildCards());
            buildEvidence();
            return;
        }
        const QJsonObject payload = doc.object();
        const QJsonArray cardsArray = payload.value("cards").toArray();
        QVector<DiagnosticCardItem> cards;
        cards.reserve(cardsArray.size());
        for (const QJsonValue &value : cardsArray) {
            const QJsonObject item = value.toObject();
            DiagnosticCardItem card;
            card.symptom = item.value("symptom").toString();
            card.cause = item.value("cause").toString();
            card.evidence = item.value("evidence").toString();
            card.action = item.value("action").toString();
            card.confidence = item.value("confidence").toDouble();
            cards.push_back(card);
        }
        if (cards.isEmpty()) {
            cards = buildCards();
        }
        m_cards.setCards(cards);

        m_evidencePoints.clear();
        const QJsonArray evidence = payload.value("evidence").toArray();
        m_evidencePoints.reserve(evidence.size());
        for (const QJsonValue &value : evidence) {
            m_evidencePoints.append(value.toDouble());
        }
        if (m_evidencePoints.isEmpty()) {
            buildEvidence();
        } else {
            emit evidencePointsChanged();
        }

        const QJsonArray checklist = payload.value("checklist").toArray();
        if (!checklist.isEmpty()) {
            m_checklist.clear();
            m_checklist.reserve(checklist.size());
            for (const QJsonValue &value : checklist) {
                m_checklist.append(value.toString());
            }
            emit checklistChanged();
        }
    });
}

void DiagnosticsViewModel::buildEvidence()
{
    m_evidencePoints.clear();
    m_evidencePoints.reserve(16);

    const uint deviceHash = qHash(m_selectedDevice);
    double base = 0.25 + (deviceHash % 5) * 0.05;
    double variance = 0.55;
    if (m_timeRange == "1h") {
        variance = 0.7;
    } else if (m_timeRange == "7d") {
        variance = 0.45;
    }
    for (int i = 0; i < 16; ++i) {
        const double jitter = (QRandomGenerator::global()->generateDouble() - 0.5) * variance;
        const double value = qBound(0.05, base + jitter, 0.95);
        m_evidencePoints.append(value);
    }
    emit evidencePointsChanged();
}
