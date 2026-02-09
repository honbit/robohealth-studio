import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "."

Rectangle {
    id: root
    height: 56
    color: Theme.surface
    border.color: Theme.border

    property string title: I18n.t("RoboHealth Studio")

    function syncLanguage() {
        if (I18n.language !== settingsVM.language) {
            I18n.language = settingsVM.language
        }
    }

    ListModel {
        id: timeModel
        ListElement { label: "Last 1h"; value: "1h" }
        ListElement { label: "Last 24h"; value: "24h" }
        ListElement { label: "Last 7d"; value: "7d" }
    }

    function indexForTimeRange(value) {
        for (var i = 0; i < timeModel.count; i++) {
            if (timeModel.get(i).value === value) {
                return i
            }
        }
        return 1
    }

    Component.onCompleted: {
        syncLanguage()
        timeCombo.currentIndex = indexForTimeRange(settingsVM.timeRange)
    }

    Connections {
        target: settingsVM
        function onLanguageChanged() { syncLanguage() }
        function onTimeRangeChanged() {
            var nextIndex = indexForTimeRange(settingsVM.timeRange)
            if (timeCombo.currentIndex !== nextIndex) {
                timeCombo.currentIndex = nextIndex
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        Label {
            text: root.title
            font.pixelSize: 18
            color: Theme.textPrimary
        }

        Badge {
            text: I18n.t("Simulated")
            tone: "accent"
            Layout.preferredWidth: 90
        }

        Item { Layout.fillWidth: true }

        ComboBox {
            id: timeCombo
            model: timeModel
            textRole: "label"
            Layout.preferredWidth: 120
            displayText: I18n.t(currentText)
            delegate: ItemDelegate { text: I18n.t(label) }
            onCurrentIndexChanged: {
                if (currentIndex >= 0) {
                    var value = timeModel.get(currentIndex).value
                    if (settingsVM.timeRange !== value) {
                        settingsVM.timeRange = value
                    }
                }
            }
        }

        LanguageToggle {
            Layout.preferredWidth: 90
            currentLanguage: settingsVM.language
            onLanguageSelected: function(value) { settingsVM.language = value }
        }
    }
}
