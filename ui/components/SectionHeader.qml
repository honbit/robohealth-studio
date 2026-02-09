import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"

RowLayout {
    id: root
    property string title: ""
    property string subtitle: ""

    Layout.fillWidth: true
    spacing: 8

    Label {
        text: root.title
        font.pixelSize: 20
        color: Theme.textPrimary
    }

    Label {
        visible: root.subtitle !== ""
        text: root.subtitle
        font.pixelSize: 12
        color: Theme.textMuted
    }

    Item { Layout.fillWidth: true }
}
