#pragma once

#include "ViewModelBase.h"

#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QString>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

struct ReportItem {
    QString title;
    QString time;
    QString summary;
};

class ReportListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        TimeRole,
        SummaryRole
    };

    explicit ReportListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setReports(const QVector<ReportItem> &reports);
    const ReportItem *itemAt(int row) const;

private:
    QVector<ReportItem> m_reports;
};

class ReportsViewModel : public ViewModelBase
{
    Q_OBJECT
    Q_PROPERTY(ReportListModel* reports READ reports CONSTANT)
    Q_PROPERTY(QString timeRange READ timeRange WRITE setTimeRange NOTIFY timeRangeChanged)
    Q_PROPERTY(QString reportType READ reportType WRITE setReportType NOTIFY reportTypeChanged)
    Q_PROPERTY(QString deviceFilter READ deviceFilter WRITE setDeviceFilter NOTIFY deviceFilterChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(QVariantMap selectedReport READ selectedReport NOTIFY selectedReportChanged)
    Q_PROPERTY(QVariantList previewMetrics READ previewMetrics NOTIFY previewMetricsChanged)
    Q_PROPERTY(QVariantList previewHighlights READ previewHighlights NOTIFY previewHighlightsChanged)

public:
    explicit ReportsViewModel(const QUrl &apiBase = QUrl(), bool useRest = false, QObject *parent = nullptr);

    ReportListModel *reports();
    QString timeRange() const;
    void setTimeRange(const QString &value);
    QString reportType() const;
    void setReportType(const QString &value);
    QString deviceFilter() const;
    void setDeviceFilter(const QString &value);
    int selectedIndex() const;
    void setSelectedIndex(int index);
    QVariantMap selectedReport() const;
    QVariantList previewMetrics() const;
    QVariantList previewHighlights() const;

    Q_INVOKABLE void preview();
    Q_INVOKABLE QString exportReport();
    Q_INVOKABLE QString exportPdf();

signals:
    void timeRangeChanged();
    void reportTypeChanged();
    void deviceFilterChanged();
    void selectedIndexChanged();
    void selectedReportChanged();
    void previewMetricsChanged();
    void previewHighlightsChanged();

private:
    void refreshReports();
    void buildLocalReports();
    void fetchReports();
    void ensureSelectionValid();
    void updatePreview();
    QVariantList buildMetrics() const;
    QVariantList buildHighlights() const;

    ReportListModel m_reports;
    QString m_timeRange = "24h";
    QString m_reportType = "Daily";
    QString m_deviceFilter;
    int m_selectedIndex = 0;
    QVariantList m_previewMetrics;
    QVariantList m_previewHighlights;
    bool m_useRest = false;
    QUrl m_apiBase;
    QNetworkAccessManager m_network;
    int m_requestId = 0;
};
