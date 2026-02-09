import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"

Rectangle {
    id: root
    property string currentLanguage: "zh"
    signal languageSelected(string value)
    height: 28
    radius: Theme.radiusSm
    color: Theme.surfaceAlt
    border.color: Theme.border

    RowLayout {
        anchors.fill: parent
        anchors.margins: 2
        spacing: 2

        Repeater {
            model: [
                { label: "中", value: "zh" },
                { label: "EN", value: "en" }
            ]

            delegate: Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 40
                radius: Theme.radiusSm
                color: root.currentLanguage === modelData.value ? Theme.accentSoft : "transparent"

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.languageSelected(modelData.value)
                }

                Label {
                    anchors.centerIn: parent
                    text: modelData.label
                    font.pixelSize: 12
                    color: root.currentLanguage === modelData.value ? Theme.accent : Theme.textMuted
                }
            }
        }
    }
}
