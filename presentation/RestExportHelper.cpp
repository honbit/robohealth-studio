#include "RestExportHelper.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QScopedPointer>
#include <QTimer>

namespace RestExport
{
QUrl resolveApiUrl(const QUrl &baseUrl, const QString &path)
{
    QUrl resolved = baseUrl;
    if (!resolved.path().endsWith('/')) {
        resolved.setPath(resolved.path() + '/');
    }
    return resolved.resolved(QUrl(path.startsWith('/') ? path.mid(1) : path));
}

Result fetchJson(QNetworkAccessManager &network, const QUrl &url, int timeoutMs)
{
    Result result;
    if (!url.isValid()) {
        result.error = QStringLiteral("invalid_url");
        return result;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = network.get(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
    } else {
        reply->abort();
        reply->deleteLater();
        result.error = QStringLiteral("timeout");
        return result;
    }

    const QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> guard(reply);
    if (reply->error() != QNetworkReply::NoError) {
        result.error = reply->errorString();
        return result;
    }

    const QByteArray data = reply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        result.error = QStringLiteral("invalid_payload");
        return result;
    }

    const QJsonObject obj = doc.object();
    const QJsonValue contentValue = obj.value(QStringLiteral("content"));
    if (!contentValue.isString()) {
        result.error = QStringLiteral("missing_content");
        return result;
    }

    result.ok = true;
    result.content = contentValue.toString().toUtf8();
    result.fileName = obj.value(QStringLiteral("fileName")).toString();
    return result;
}
}
