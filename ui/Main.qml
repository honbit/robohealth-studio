import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "theme"
import "components"
import "state"

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 800
    title: I18n.t("RoboHealth Studio")

    property string currentPage: "Dashboard"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TopBar {
            Layout.fillWidth: true
            title: I18n.t("RoboHealth Studio")
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Sidebar {
                id: sidebar
                Layout.preferredWidth: 220
                Layout.fillHeight: true
                currentPage: root.currentPage
                onPageSelected: (pageId) => {
                    root.currentPage = pageId
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Theme.background

                Loader {
                    id: pageLoader
                    anchors.fill: parent
                    source: {
                        switch (root.currentPage) {
                        case "Dashboard": return "pages/DashboardPage.qml"
                        case "Devices": return "pages/DevicesPage.qml"
                        case "Alerts": return "pages/AlertsPage.qml"
                        case "Logs": return "pages/LogsPage.qml"
                        case "Replay": return "pages/ReplayPage.qml"
                        case "Diagnostics": return "pages/DiagnosticsPage.qml"
                        case "Reports": return "pages/ReportsPage.qml"
                        case "Settings": return "pages/SettingsPage.qml"
                        default: return "pages/DashboardPage.qml"
                        }
                    }
                }
            }
        }
    }
}
