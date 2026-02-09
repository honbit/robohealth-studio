import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "../components"

Page {
    id: root
    title: I18n.t("Dashboard")

    property var filteredTrendPoints: []

    function updateTrendPoints() {
        var points = dashboardVM.trendPoints
        if (!points || points.length === 0) {
            filteredTrendPoints = []
            return
        }
        if (settingsVM.timeRange === "1h") {
            filteredTrendPoints = points.slice(Math.max(0, points.length - 8))
        } else if (settingsVM.timeRange === "7d") {
            filteredTrendPoints = points.slice(Math.max(0, points.length - 12))
        } else {
            filteredTrendPoints = points
        }
    }

    Component.onCompleted: updateTrendPoints()

    Connections {
        target: settingsVM
        function onTimeRangeChanged() {
            updateTrendPoints()
        }
    }

    Connections {
        target: dashboardVM
        function onTrendPointsChanged() {
            updateTrendPoints()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.background

        ScrollView {
            anchors.fill: parent
            contentWidth: parent.width

            ColumnLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 24
                spacing: 16

                SectionHeader {
                    title: I18n.t("Dashboard")
                    subtitle: I18n.t("System overview")
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: I18n.t("Overview")
                        font.pixelSize: 12
                        color: Theme.textMuted
                    }

                    Item { Layout.fillWidth: true }

                    Label { text: I18n.t("Updated just now") + " · " + dashboardVM.lastUpdated; color: Theme.textMuted; font.pixelSize: 12 }
                    GhostButton { text: I18n.t("Refresh"); onClicked: dashboardVM.refresh() }
                }

                GridLayout {
                    id: kpiGrid
                    columns: 5
                    columnSpacing: 12
                    rowSpacing: 12
                    Layout.fillWidth: true

                    KpiCard { Layout.preferredHeight: 72; Layout.fillWidth: true; label: "Temperature"; value: dashboardVM.temperature.toFixed(1); unit: "C" }
                    KpiCard {
                        Layout.preferredHeight: 72
                        Layout.fillWidth: true
                        label: "Vibration"
                        value: dashboardVM.vibration.toFixed(2)
                        unit: "g"
                        alerted: dashboardVM.vibration > settingsVM.vibrationThreshold
                    }
                    KpiCard {
                        Layout.preferredHeight: 72
                        Layout.fillWidth: true
                        label: "Current"
                        value: dashboardVM.current.toFixed(1)
                        unit: "A"
                        alerted: dashboardVM.current > settingsVM.currentThreshold
                    }
                    KpiCard {
                        Layout.preferredHeight: 72
                        Layout.fillWidth: true
                        label: "Torque"
                        value: dashboardVM.torque.toFixed(1)
                        unit: "N*m"
                        alerted: dashboardVM.torque > settingsVM.torqueThreshold
                    }
                    KpiCard { Layout.preferredHeight: 72; Layout.fillWidth: true; label: "Error Rate"; value: dashboardVM.errorRate.toFixed(1); unit: "%" }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    TrendChart {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 260
                        points: filteredTrendPoints
                        alerted: dashboardVM.vibration > settingsVM.vibrationThreshold
                    }
                    DeviceStatusList { Layout.preferredWidth: 320; Layout.preferredHeight: 260; modelSource: devicesVM.devices }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    AlertList { Layout.fillWidth: true; Layout.preferredHeight: 220; modelSource: alertsVM.alerts }
                    Timeline { Layout.preferredWidth: 320; Layout.preferredHeight: 220; modelSource: logsVM.logs }
                }

                Item { Layout.fillHeight: true }
            }
        }
    }
}
