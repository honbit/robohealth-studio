#pragma once

#include "ViewModelBase.h"

#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <QVector>

struct DiagnosticCardItem {
    QString symptom;
    QString cause;
    QString evidence;
    QString action;
    double confidence = 0.0;
};

class DiagnosticCardModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        SymptomRole = Qt::UserRole + 1,
        CauseRole,
        EvidenceRole,
        ActionRole,
        ConfidenceRole
    };

    explicit DiagnosticCardModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setCards(const QVector<DiagnosticCardItem> &cards);
    const DiagnosticCardItem *itemAt(int row) const;

private:
    QVector<DiagnosticCardItem> m_cards;
};

class DiagnosticsViewModel : public ViewModelBase
{
    Q_OBJECT
    Q_PROPERTY(DiagnosticCardModel* cards READ cards CONSTANT)
    Q_PROPERTY(QVariantList evidencePoints READ evidencePoints NOTIFY evidencePointsChanged)
    Q_PROPERTY(QStringList checklist READ checklist NOTIFY checklistChanged)
    Q_PROPERTY(QString selectedDevice READ selectedDevice WRITE setSelectedDevice NOTIFY selectedDeviceChanged)
    Q_PROPERTY(QString timeRange READ timeRange WRITE setTimeRange NOTIFY timeRangeChanged)

public:
    explicit DiagnosticsViewModel(const QUrl &apiBase = QUrl(), bool useRest = false, QObject *parent = nullptr);

    DiagnosticCardModel *cards();
    QVariantList evidencePoints() const;
    QStringList checklist() const;
    QString selectedDevice() const;
    QString timeRange() const;

    void setSelectedDevice(const QString &value);
    void setTimeRange(const QString &value);

    Q_INVOKABLE void generate();

signals:
    void evidencePointsChanged();
    void checklistChanged();
    void selectedDeviceChanged();
    void timeRangeChanged();

private:
    void fetchDiagnostics();
    void buildEvidence();
    QVector<DiagnosticCardItem> buildCards() const;

    DiagnosticCardModel m_cards;
    QVariantList m_evidencePoints;
    QStringList m_checklist;
    QString m_selectedDevice = "Robot-01";
    QString m_timeRange = "24h";
    bool m_useRest = false;
    QUrl m_apiBase;
    QNetworkAccessManager m_network;
    int m_requestId = 0;
};
