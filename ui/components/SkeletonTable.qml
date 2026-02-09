import QtQuick
import QtQuick.Layouts
import "../theme"

Item {
    id: root
    property int rows: 6
    property var columnRatios: [0.2, 0.5, 0.3]
    property int rowHeight: 32
    property int rowSpacing: 6
    property int rowRadius: Theme.radiusSm

    ColumnLayout {
        anchors.fill: parent
        spacing: root.rowSpacing

        Repeater {
            model: root.rows

            Rectangle {
                Layout.fillWidth: true
                height: root.rowHeight
                radius: root.rowRadius
                color: Theme.surfaceAlt
                border.color: Theme.border

                Row {
                    id: bars
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Repeater {
                        model: root.columnRatios
                        Rectangle {
                            height: bars.height
                            width: Math.max(24, (bars.width - (root.columnRatios.length - 1) * bars.spacing) * modelData)
                            radius: 4
                            color: Theme.surface
                        }
                    }
                }
            }
        }
    }

    SequentialAnimation on opacity {
        loops: Animation.Infinite
        NumberAnimation { from: 1.0; to: 0.6; duration: 700; easing.type: Easing.InOutQuad }
        NumberAnimation { from: 0.6; to: 1.0; duration: 700; easing.type: Easing.InOutQuad }
    }
}
