import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "."

Card {
    id: root
    property var modelSource: null

    ListModel {
        id: timelineModel
        ListElement { time: "10:45"; text: "Replay started" }
        ListElement { time: "10:42"; text: "Critical alert acknowledged" }
        ListElement { time: "10:37"; text: "Vibration warning" }
        ListElement { time: "10:20"; text: "Camera reconnected" }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label { text: I18n.t("Timeline"); color: Theme.textMuted; font.pixelSize: 12 }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.modelSource ? root.modelSource : timelineModel
            spacing: 6

            delegate: Rectangle {
                width: ListView.view ? ListView.view.width : 0
                height: 32
                radius: Theme.radiusSm
                color: Theme.surfaceAlt
                border.color: Theme.border

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 8

                    Label { text: time; color: Theme.textMuted; Layout.preferredWidth: 44 }
                    Label {
                        property string entryText: (text !== undefined ? text : (message !== undefined ? message : ""))
                        text: I18n.t(entryText)
                        color: Theme.textPrimary
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
