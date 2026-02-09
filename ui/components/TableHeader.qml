import QtQuick
import QtQuick.Layouts
import "../theme"

Rectangle {
    id: root
    property int padding: 10
    default property alias content: headerRow.data

    height: 34
    radius: Theme.radiusSm
    color: Theme.surfaceAlt
    border.color: Theme.border
    border.width: 1
    clip: true

    RowLayout {
        id: headerRow
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: 8
    }
}
