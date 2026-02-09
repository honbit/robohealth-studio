import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "."

Card {
    id: root
    property string label: "KPI"
    property string value: "--"
    property string unit: ""
    property bool alerted: false

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        width: parent.width
        height: 3
        color: root.alerted ? Theme.danger : Theme.accent
        radius: Theme.radiusMd
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Label { text: I18n.t(root.label); color: Theme.textMuted; font.pixelSize: 12 }
        Label {
            text: root.value + (root.unit !== "" ? (" " + root.unit) : "")
            color: root.alerted ? Theme.danger : Theme.textPrimary
            font.pixelSize: 20
        }
    }
}
