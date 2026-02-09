import QtQuick
import "../theme"

Rectangle {
    id: root
    property int padding: 12
    default property alias content: contentItem.data

    radius: Theme.radiusMd
    color: Theme.surface
    border.color: Theme.border
    border.width: 1

    Item {
        id: contentItem
        anchors.fill: parent
        anchors.margins: root.padding
    }
}
