import QtQuick
import QtQuick.Controls
import "../theme"

Item {
    id: root
    property string text: ""
    property int timeout: 2000
    property int maxWidth: 420
    property int horizontalPadding: 16
    property int verticalPadding: 10
    property int bottomMargin: 24
    property bool opened: false

    implicitWidth: Math.min(maxWidth, label.implicitWidth + horizontalPadding * 2)
    implicitHeight: label.implicitHeight + verticalPadding * 2
    width: implicitWidth
    height: implicitHeight
    opacity: opened ? 1 : 0
    visible: opacity > 0
    z: 1000

    function open() {
        opened = true
        hideTimer.restart()
    }

    function close() {
        opened = false
    }

    Timer {
        id: hideTimer
        interval: root.timeout
        repeat: false
        onTriggered: root.close()
    }

    Behavior on opacity {
        NumberAnimation { duration: 160 }
    }

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusSm
        color: Theme.surface
        border.color: Theme.border
    }

    Label {
        id: label
        anchors.fill: parent
        anchors.margins: root.horizontalPadding
        text: root.text
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: Theme.textPrimary
    }

    onOpenedChanged: {
        if (opened) {
            hideTimer.restart()
        }
    }

    y: parent ? parent.height - height - bottomMargin : 0
}
