import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "."

Item {
    id: root
    property string title: ""
    property string subtitle: ""
    property bool compact: false
    property string actionText: ""

    signal actionTriggered()

    implicitWidth: 200
    implicitHeight: compact ? 72 : 96

    ColumnLayout {
        anchors.centerIn: parent
        spacing: root.compact ? 4 : 6

        Rectangle {
            width: root.compact ? 24 : 32
            height: width
            radius: width / 2
            color: Theme.surfaceAlt
            border.color: Theme.border

            Column {
                anchors.centerIn: parent
                spacing: root.compact ? 2 : 3

                Repeater {
                    model: 3
                    Rectangle {
                        width: root.compact ? 12 : 16
                        height: 2
                        radius: 1
                        color: Theme.textMuted
                    }
                }
            }
        }

        Label {
            text: root.title
            color: Theme.textMuted
            font.pixelSize: root.compact ? 12 : 13
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            visible: root.subtitle.length > 0
            text: root.subtitle
            color: Theme.textMuted
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignHCenter
        }

        GhostButton {
            visible: root.actionText.length > 0
            text: root.actionText
            font.pixelSize: 11
            padding: 6
            implicitHeight: 28
            onClicked: root.actionTriggered()
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
