#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include "AlertsViewModel.h"
#include "DashboardViewModel.h"
#include "DevicesViewModel.h"
#include "DiagnosticsViewModel.h"
#include "LogsViewModel.h"
#include "ReplayViewModel.h"
#include "ReportsViewModel.h"
#include "SettingsViewModel.h"
#include "TelemetryServiceBase.h"
#include "MockTelemetryService.h"
#include "RestTelemetryService.h"

#include <memory>

int main(int argc, char *argv[])
{
    QQuickStyle::setStyle("Basic");
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("RoboHealth");
    QCoreApplication::setApplicationName("RoboHealth Studio");

    const QByteArray platform = qgetenv("QT_QPA_PLATFORM");
    const bool headlessMode = (platform == "offscreen" || platform == "minimal");

    const QByteArray backend = qgetenv("ROBOHEALTH_BACKEND");
    const QByteArray apiUrlEnv = qgetenv("ROBOHEALTH_API_URL");
    const bool useRest = (backend == "rest") || !apiUrlEnv.isEmpty();
    const QUrl apiUrl = apiUrlEnv.isEmpty()
        ? QUrl(QStringLiteral("http://127.0.0.1:5000"))
        : QUrl(QString::fromUtf8(apiUrlEnv));

    std::unique_ptr<TelemetryServiceBase> telemetryService;
    if (useRest) {
        telemetryService = std::make_unique<RestTelemetryService>(apiUrl);
    } else {
        telemetryService = std::make_unique<MockTelemetryService>();
    }

    DashboardViewModel dashboardViewModel(telemetryService.get());
    DevicesViewModel devicesViewModel(telemetryService.get());
    AlertsViewModel alertsViewModel(telemetryService.get(), apiUrl, useRest);
    LogsViewModel logsViewModel(telemetryService.get(), apiUrl, useRest);
    ReplayViewModel replayViewModel(apiUrl, useRest);
    DiagnosticsViewModel diagnosticsViewModel(apiUrl, useRest);
    ReportsViewModel reportsViewModel(apiUrl, useRest);
    SettingsViewModel settingsViewModel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("headlessMode", headlessMode);
    engine.rootContext()->setContextProperty("dashboardVM", &dashboardViewModel);
    engine.rootContext()->setContextProperty("devicesVM", &devicesViewModel);
    engine.rootContext()->setContextProperty("alertsVM", &alertsViewModel);
    engine.rootContext()->setContextProperty("logsVM", &logsViewModel);
    engine.rootContext()->setContextProperty("replayVM", &replayViewModel);
    engine.rootContext()->setContextProperty("diagnosticsVM", &diagnosticsViewModel);
    engine.rootContext()->setContextProperty("reportsVM", &reportsViewModel);
    engine.rootContext()->setContextProperty("settingsVM", &settingsViewModel);
    engine.load(QUrl(QStringLiteral("qrc:/ui/Main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
