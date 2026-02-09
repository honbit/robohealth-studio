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
        id: deviceModel
        ListElement { name: "Robot-01"; status: "online"; health: 92 }
        ListElement { name: "Robot-02"; status: "online"; health: 88 }
        ListElement { name: "Cam-04"; status: "offline"; health: 61 }
        ListElement { name: "PLC-01"; status: "online"; health: 90 }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label { text: I18n.t("Device Status"); color: Theme.textMuted; font.pixelSize: 12 }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.modelSource ? root.modelSource : deviceModel
            spacing: 6

            delegate: Rectangle {
                width: ListView.view ? ListView.view.width : 0
                height: 36
                radius: Theme.radiusSm
                color: Theme.surfaceAlt
                border.color: Theme.border

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: status === "Online" || status === "online" ? "#2ECC71" : "#95A5A6"
                    }

                    Label { text: name; Layout.fillWidth: true; color: Theme.textPrimary }
                    Label {
                        text: health !== undefined
                            ? (I18n.t("Health") + " " + health + "%")
                            : (I18n.language === "zh" ? (alerts + " 条告警") : (alerts + " alerts"))
                        color: Theme.textMuted
                    }
                }
            }
        }
    }
}
