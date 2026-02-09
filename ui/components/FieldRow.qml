import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"

RowLayout {
    id: root
    property string label: ""
    property string value: ""
    property bool multiline: false
    property int labelWidth: 90
    property color valueColor: Theme.textPrimary

    spacing: 8

    Label {
        text: root.label
        color: Theme.textMuted
        font.pixelSize: 12
        Layout.preferredWidth: root.labelWidth
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        text: root.value
        color: root.valueColor
        font.pixelSize: 12
        Layout.fillWidth: true
        wrapMode: root.multiline ? Text.Wrap : Text.NoWrap
        verticalAlignment: Text.AlignVCenter
    }
}
