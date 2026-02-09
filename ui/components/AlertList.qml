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
        id: alertModel
        ListElement { level: "critical"; message: "Joint torque spike"; device: "Robot-02"; time: "10:42" }
        ListElement { level: "warn"; message: "Vibration above threshold"; device: "Robot-01"; time: "10:37" }
        ListElement { level: "info"; message: "Camera reconnect"; device: "Cam-04"; time: "10:20" }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label { text: I18n.t("Recent Alerts"); color: Theme.textMuted; font.pixelSize: 12 }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.modelSource ? root.modelSource : alertModel
            spacing: 6

            delegate: Rectangle {
                width: ListView.view ? ListView.view.width : 0
                height: 40
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
                        color: {
                            var normalized = String(level).toLowerCase()
                            return normalized === "critical" ? "#E74C3C" : (normalized === "warn" ? "#F39C12" : "#2ECC71")
                        }
                    }

                    Label { text: I18n.t(message); Layout.fillWidth: true; color: Theme.textPrimary; elide: Text.ElideRight }
                    Label { text: device; color: Theme.textMuted }
                    Label { text: time; color: Theme.textMuted }
                }
            }
        }
    }
}
