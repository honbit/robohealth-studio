#pragma once

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QString>
#include <QUrl>

namespace RestExport
{
struct Result {
    bool ok = false;
    QString fileName;
    QByteArray content;
    QString error;
};

QUrl resolveApiUrl(const QUrl &baseUrl, const QString &path);
Result fetchJson(QNetworkAccessManager &network, const QUrl &url, int timeoutMs = 2000);
}
