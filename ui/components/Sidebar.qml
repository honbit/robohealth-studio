import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"

Rectangle {
    id: root
    color: Theme.background
    border.color: Theme.border

    signal pageSelected(string pageId)
    property string currentPage: "Dashboard"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 4

        Label {
            text: I18n.t("RoboHealth")
            font.pixelSize: 18
            color: Theme.textPrimary
        }

        Repeater {
            model: [
                { id: "Dashboard", label: "Dashboard" },
                { id: "Devices", label: "Devices" },
                { id: "Alerts", label: "Alerts" },
                { id: "Logs", label: "Logs" },
                { id: "Replay", label: "Replay" },
                { id: "Diagnostics", label: "Diagnostics" },
                { id: "Reports", label: "Reports" },
                { id: "Settings", label: "Settings" }
            ]

            delegate: Rectangle {
                Layout.fillWidth: true
                height: 36
                radius: 6
                color: root.currentPage === modelData.id ? Theme.accentSoft : "transparent"
                border.color: root.currentPage === modelData.id ? Theme.accent : "transparent"

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.pageSelected(modelData.id)
                }

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    text: I18n.t(modelData.label)
                    color: root.currentPage === modelData.id ? Theme.textPrimary : Theme.textMuted
                    font.pixelSize: 13
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
