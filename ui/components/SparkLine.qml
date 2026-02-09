import QtQuick
import QtCharts
import "../theme"

Item {
    id: root
    property var points: [0.1, 0.15, 0.22, 0.18, 0.26, 0.42, 0.38, 0.55]
    property bool alerted: false
    property bool headless: (typeof headlessMode !== "undefined" && headlessMode)

    function rebuildChart() {
        if (!chartLoader.item || !chartLoader.item.rebuild) {
            return
        }
        chartLoader.item.rebuild()
    }

    Loader {
        id: chartLoader
        anchors.fill: parent
        sourceComponent: root.headless ? placeholderComponent : chartComponent
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
                    tickCount: 2
                    labelsVisible: false
                    gridVisible: false
                    lineVisible: false
                    minorGridVisible: false
                }

                ValueAxis {
                    id: axisY
                    min: 0
                    max: 1
                    tickCount: 2
                    labelsVisible: false
                    gridVisible: false
                    lineVisible: false
                    minorGridVisible: false
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
