import QtQuick
import QtQuick.Controls
import "../theme"

Button {
    id: root
    font.pixelSize: 12
    padding: 8
    implicitHeight: 32

    background: Rectangle {
        radius: Theme.radiusSm
        color: root.enabled ? (root.down ? "#E07A00" : Theme.accent) : Theme.border
        border.color: root.enabled ? Theme.accent : Theme.border
    }

    contentItem: Text {
        text: root.text
        color: root.enabled ? "white" : Theme.textMuted
        font.pixelSize: 12
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
