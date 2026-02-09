import QtQuick
import QtQuick.Layouts
import "../theme"

Rectangle {
    id: root
    property bool selected: false
    property bool hoverable: true
    signal clicked()

    default property alias content: contentRow.data

    height: 40
    radius: Theme.radiusSm
    color: selected ? Theme.accentSoft : (hoverable && mouseArea.containsMouse ? Theme.surface : Theme.surfaceAlt)
    border.color: selected ? Theme.accent : Theme.border
    border.width: 1
    clip: true

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    RowLayout {
        id: contentRow
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
    }
}
