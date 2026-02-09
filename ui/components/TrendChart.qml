import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts
import "../theme"
import "../state"
import "."

Card {
    id: root
    property var points: [0.1, 0.15, 0.22, 0.18, 0.26, 0.42, 0.38, 0.55, 0.46, 0.62]
    property bool alerted: false
    property bool headless: (typeof headlessMode !== "undefined" && headlessMode)

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label { text: I18n.t("Trend"); color: Theme.textMuted; font.pixelSize: 12 }
            Item { Layout.fillWidth: true }
            ComboBox { model: ["Robot-01", "Robot-02", "Robot-03"]; Layout.preferredWidth: 110 }
            ComboBox { model: [I18n.t("Temperature"), I18n.t("Vibration"), I18n.t("Current")]; Layout.preferredWidth: 120 }
        }

        Loader {
            id: chartLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: root.headless ? placeholderComponent : chartComponent
        }
    }

    function rebuildChart() {
        if (!chartLoader.item || !chartLoader.item.rebuild) {
            return
        }
        chartLoader.item.rebuild()
    }

    onPointsChanged: rebuildChart()

    Component.onCompleted: rebuildChart()

    Component {
        id: chartComponent
        Item {
            id: chartWrapper
            anchors.fill: parent

            function rebuild() {
                series.clear()
                for (var i = 0; i < root.points.length; i++) {
                    series.append(i, root.points[i])
                }
                axisX.max = Math.max(1, root.points.length - 1)
            }

            ChartView {
                id: chart
                anchors.fill: parent
                antialiasing: true
                legend.visible: false
                backgroundColor: "transparent"

                ValueAxis {
                    id: axisX
                    min: 0
                    max: root.points.length - 1
                    tickCount: 5
                    labelFormat: "%.0f"
                    labelsColor: Theme.textMuted
                    gridLineColor: Theme.border
                }

                ValueAxis {
                    id: axisY
                    min: 0
                    max: 1
                    tickCount: 5
                    labelFormat: "%.1f"
                    labelsColor: Theme.textMuted
                    gridLineColor: Theme.border
                }

                LineSeries {
                    id: series
                    axisX: axisX
                    axisY: axisY
                    color: root.alerted ? Theme.danger : Theme.accent
                    width: 2
                }
            }

            Component.onCompleted: rebuild()
        }
    }

    Component {
        id: placeholderComponent
        Rectangle {
            anchors.fill: parent
            radius: Theme.radiusSm
            color: Theme.surfaceAlt
            border.color: Theme.border
        }
    }
}
