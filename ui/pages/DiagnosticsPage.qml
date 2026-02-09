import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "../components"

Page {
    id: root
    title: I18n.t("Diagnostics")

    property bool listLoading: true

    function indexForTimeRange(value) {
        for (var i = 0; i < timeModel.count; i++) {
            if (timeModel.get(i).value === value) {
                return i
            }
        }
        return 1
    }

    function indexForDevice(value) {
        for (var i = 0; i < deviceModel.count; i++) {
            if (deviceModel.get(i).value === value) {
                return i
            }
        }
        return 0
    }

    function triggerLoading() {
        listLoading = true
        loadingTimer.restart()
    }

    function stopLoading() {
        listLoading = false
        loadingTimer.stop()
    }

    Component.onCompleted: {
        deviceCombo.currentIndex = indexForDevice(diagnosticsVM.selectedDevice)
        timeCombo.currentIndex = indexForTimeRange(settingsVM.timeRange)
        if (deviceCombo.currentIndex >= 0) {
            diagnosticsVM.selectedDevice = deviceModel.get(deviceCombo.currentIndex).value
        }
        if (timeCombo.currentIndex >= 0) {
            diagnosticsVM.timeRange = timeModel.get(timeCombo.currentIndex).value
        }
        triggerLoading()
    }

    Timer {
        id: loadingTimer
        interval: 700
        repeat: false
        running: true
        onTriggered: listLoading = false
    }

    Connections {
        target: diagnosticsVM.cards
        function onModelReset() { stopLoading() }
        function onRowsInserted() { stopLoading() }
        function onRowsRemoved() { stopLoading() }
        function onLayoutChanged() { stopLoading() }
    }

    Connections {
        target: settingsVM
        function onTimeRangeChanged() {
            var nextIndex = indexForTimeRange(settingsVM.timeRange)
            if (timeCombo.currentIndex !== nextIndex) {
                timeCombo.currentIndex = nextIndex
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.background

        ScrollView {
            anchors.fill: parent
            contentWidth: parent.width

            ColumnLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 24
                spacing: 16

                SectionHeader { title: I18n.t("Diagnostics"); subtitle: I18n.t("Explain and suggest") }

                ListModel {
                    id: deviceModel
                    ListElement { label: "Robot-01"; value: "Robot-01" }
                    ListElement { label: "Robot-02"; value: "Robot-02" }
                    ListElement { label: "Cam-04"; value: "Cam-04" }
                }

                ListModel {
                    id: timeModel
                    ListElement { label: "Last 1h"; value: "1h" }
                    ListElement { label: "Last 24h"; value: "24h" }
                    ListElement { label: "Last 7d"; value: "7d" }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ComboBox {
                        id: deviceCombo
                        Layout.preferredWidth: 160
                        model: deviceModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                diagnosticsVM.selectedDevice = deviceModel.get(currentIndex).value
                                diagnosticsVM.generate()
                                triggerLoading()
                            }
                        }
                    }
                    ComboBox {
                        id: timeCombo
                        Layout.preferredWidth: 140
                        model: timeModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                var value = timeModel.get(currentIndex).value
                                diagnosticsVM.timeRange = value
                                if (settingsVM.timeRange !== value) {
                                    settingsVM.timeRange = value
                                }
                                diagnosticsVM.generate()
                                triggerLoading()
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    GhostButton { text: I18n.t("Refresh"); onClicked: diagnosticsVM.generate() }
                    PrimaryButton { text: I18n.t("Generate"); onClicked: diagnosticsVM.generate() }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Card {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 360

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8

                            Label { text: I18n.t("Diagnosis Cards"); color: Theme.textMuted; font.pixelSize: 12 }

                            ListView {
                                id: cardList
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                model: diagnosticsVM.cards
                                spacing: 6
                                visible: !listLoading && count > 0

                                delegate: Rectangle {
                                    width: ListView.view ? ListView.view.width : 0
                                    height: 64
                                    radius: Theme.radiusSm
                                    color: Theme.surfaceAlt
                                    border.color: Theme.border

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 2

                                        RowLayout {
                                            Layout.fillWidth: true
                                            Label { text: I18n.t(symptom); color: Theme.textPrimary; Layout.fillWidth: true }
                                            Badge { text: "C " + Number(confidence).toFixed(2); tone: "accent"; Layout.preferredWidth: 60 }
                                        }
                                        Label { text: I18n.t("Cause: ") + I18n.t(cause); color: Theme.textMuted }
                                        Label { text: I18n.t("Action: ") + I18n.t(action); color: Theme.textMuted }
                                    }
                                }
                            }

                            SkeletonTable {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: listLoading
                                rows: 3
                                rowHeight: 64
                                rowSpacing: 8
                                columnRatios: [0.7, 0.3]
                            }

                            Label {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: !listLoading && cardList.count === 0
                                text: I18n.t("No data")
                                color: Theme.textMuted
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }

                    Card {
                        Layout.preferredWidth: 320
                        Layout.preferredHeight: 360

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8

                            Label { text: I18n.t("Evidence"); color: Theme.textMuted; font.pixelSize: 12 }
                            TrendChart { Layout.fillWidth: true; Layout.preferredHeight: 200; points: diagnosticsVM.evidencePoints }

                            Label { text: I18n.t("Checklist"); color: Theme.textMuted; font.pixelSize: 12 }

                            ColumnLayout {
                                spacing: 6
                                Repeater {
                                    model: diagnosticsVM.checklist
                                    delegate: CheckBox { text: I18n.t(modelData) }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
