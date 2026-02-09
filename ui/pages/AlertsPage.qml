import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "../components"

Page {
    id: root
    title: I18n.t("Alerts")

    property var alertDetail: alertsVM.selectedAlert
    property bool listLoading: true
    property bool hasAlertSelection: alertsVM.selectedIndex >= 0 && alertDetail.level
    property bool alertAcknowledged: !!(alertDetail && alertDetail.acknowledged)
    property bool alertMuted: !!(alertDetail && alertDetail.muted)

    function indexForTimeRange(value) {
        for (var i = 0; i < timeModel.count; i++) {
            if (timeModel.get(i).value === value) {
                return i
            }
        }
        return 1
    }

    function triggerLoading() {
        listLoading = true
        loadingTimer.restart()
    }

    function stopLoading() {
        listLoading = false
        loadingTimer.stop()
    }

    function alertFilterSummary() {
        var parts = []
        if (levelCombo.currentIndex > 0) {
            parts.push(I18n.t("Level") + ": " + I18n.t(levelCombo.currentText))
        }
        if (deviceCombo.currentIndex > 0) {
            parts.push(I18n.t("Device") + ": " + I18n.t(deviceCombo.currentText))
        }
        if (timeCombo.currentIndex > 0) {
            parts.push(I18n.t("Time") + ": " + I18n.t(timeCombo.currentText))
        }
        if (statusCombo.currentIndex > 0) {
            parts.push(I18n.t("Status") + ": " + I18n.t(statusCombo.currentText))
        }
        return parts.join(" · ")
    }

    function resetFilters() {
        levelCombo.currentIndex = 0
        deviceCombo.currentIndex = 0
        timeCombo.currentIndex = 1
        statusCombo.currentIndex = 0
        triggerLoading()
    }

    Component.onCompleted: {
        if (levelCombo.currentIndex >= 0) {
            alertsVM.filteredAlerts.levelFilter = levelModel.get(levelCombo.currentIndex).value
        }
        if (deviceCombo.currentIndex >= 0) {
            alertsVM.filteredAlerts.deviceFilter = deviceModel.get(deviceCombo.currentIndex).value
        }
        timeCombo.currentIndex = indexForTimeRange(settingsVM.timeRange)
        if (timeCombo.currentIndex >= 0) {
            alertsVM.filteredAlerts.timeRange = timeModel.get(timeCombo.currentIndex).value
        }
        if (statusCombo.currentIndex >= 0) {
            alertsVM.filteredAlerts.statusFilter = statusModel.get(statusCombo.currentIndex).value
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
        target: alertsVM.filteredAlerts
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

    Snackbar {
        id: alertToast
        timeout: 2000
        anchors.horizontalCenter: parent.horizontalCenter
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

                SectionHeader {
                    title: I18n.t("Alerts")
                    subtitle: I18n.t("Active and historical")
                }

                ListModel {
                    id: levelModel
                    ListElement { label: "All Levels"; value: "" }
                    ListElement { label: "Critical"; value: "Critical" }
                    ListElement { label: "Warn"; value: "Warn" }
                    ListElement { label: "Info"; value: "Info" }
                }

                ListModel {
                    id: deviceModel
                    ListElement { label: "All Devices"; value: "" }
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

                ListModel {
                    id: statusModel
                    ListElement { label: "All Status"; value: "" }
                    ListElement { label: "Active"; value: "active" }
                    ListElement { label: "Acknowledged"; value: "acknowledged" }
                    ListElement { label: "Muted"; value: "muted" }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ComboBox {
                        id: levelCombo
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 32
                        font.pixelSize: 12
                        model: levelModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                alertsVM.filteredAlerts.levelFilter = levelModel.get(currentIndex).value
                                triggerLoading()
                            }
                        }
                    }
                    ComboBox {
                        id: deviceCombo
                        Layout.preferredWidth: 140
                        Layout.preferredHeight: 32
                        font.pixelSize: 12
                        model: deviceModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                alertsVM.filteredAlerts.deviceFilter = deviceModel.get(currentIndex).value
                                triggerLoading()
                            }
                        }
                    }
                    ComboBox {
                        id: timeCombo
                        Layout.preferredWidth: 140
                        Layout.preferredHeight: 32
                        font.pixelSize: 12
                        model: timeModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                var value = timeModel.get(currentIndex).value
                                alertsVM.filteredAlerts.timeRange = value
                                if (settingsVM.timeRange !== value) {
                                    settingsVM.timeRange = value
                                }
                                triggerLoading()
                            }
                        }
                    }
                    ComboBox {
                        id: statusCombo
                        Layout.preferredWidth: 140
                        Layout.preferredHeight: 32
                        font.pixelSize: 12
                        model: statusModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                alertsVM.filteredAlerts.statusFilter = statusModel.get(currentIndex).value
                                triggerLoading()
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    GhostButton {
                        text: I18n.t("Reset")
                        onClicked: resetFilters()
                    }
                    PrimaryButton {
                        text: I18n.t("Export")
                        onClicked: {
                            var path = alertsVM.exportAlerts()
                            if (path) {
                                alertToast.text = I18n.t("Exported to: ") + path + " " + I18n.t("(mock)")
                            } else {
                                alertToast.text = I18n.t("Export failed")
                            }
                            alertToast.open()
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: alertFilterSummary().length > 0
                    text: I18n.t("Filters: ") + alertFilterSummary()
                    color: Theme.textMuted
                    font.pixelSize: 12
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Card {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 420

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8

                            TableHeader {
                                Layout.fillWidth: true
                                HeaderLabel { text: I18n.t("Level"); Layout.preferredWidth: 80 }
                                HeaderLabel { text: I18n.t("Message"); Layout.fillWidth: true }
                                HeaderLabel { text: I18n.t("Device"); Layout.preferredWidth: 90 }
                                HeaderLabel { text: I18n.t("Time"); Layout.preferredWidth: 70 }
                                HeaderLabel { text: I18n.t("Status"); Layout.preferredWidth: 90 }
                            }

                            ListView {
                                id: alertList
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                model: alertsVM.filteredAlerts
                                spacing: 4
                                currentIndex: alertsVM.selectedIndex
                                visible: !listLoading && count > 0
                                clip: true
                                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                                delegate: DataRow {
                                    width: ListView.view ? ListView.view.width : 0
                                    selected: ListView.isCurrentItem

                                    onClicked: {
                                        alertList.currentIndex = index
                                        alertsVM.selectedIndex = index
                                    }

                                    Badge {
                                        text: I18n.t(level)
                                        tone: level === "Critical" ? "warn" : (level === "Warn" ? "accent" : "muted")
                                        Layout.preferredWidth: 72
                                    }
                                    Label { text: I18n.t(message); color: Theme.textPrimary; Layout.fillWidth: true }
                                    Label { text: device; color: Theme.textMuted; Layout.preferredWidth: 90 }
                                    Label { text: time; color: Theme.textMuted; Layout.preferredWidth: 70 }
                                    Badge {
                                        text: muted ? I18n.t("Muted") : (acknowledged ? I18n.t("Acknowledged") : I18n.t("Active"))
                                        tone: muted ? "muted" : (acknowledged ? "accent" : "warn")
                                        Layout.preferredWidth: 90
                                    }
                                }
                            }

                            SkeletonTable {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: listLoading
                                rows: 6
                                columnRatios: [0.18, 0.46, 0.16, 0.1, 0.1]
                            }

                            EmptyState {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: !listLoading && alertList.count === 0
                                title: alertFilterSummary().length > 0 ? I18n.t("No results") : I18n.t("No data")
                                subtitle: alertFilterSummary().length > 0 ? I18n.t("Try clearing filters") : ""
                                actionText: alertFilterSummary().length > 0 ? I18n.t("Clear filters") : ""
                                onActionTriggered: resetFilters()
                                compact: true
                            }
                        }
                    }

                    Card {
                        Layout.preferredWidth: 320
                        Layout.preferredHeight: 420

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10

                            Label { text: I18n.t("Alert Detail"); color: Theme.textPrimary; font.pixelSize: 14 }
                            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

                            EmptyState {
                                Layout.fillWidth: true
                                visible: !hasAlertSelection
                                title: I18n.t("Select an alert to view details")
                                subtitle: I18n.t("Select from the list")
                            }

                            Badge {
                                visible: hasAlertSelection
                                text: alertDetail.level ? I18n.t(alertDetail.level) : "-"
                                tone: alertDetail.level === "Critical" ? "warn" : "muted"
                                Layout.preferredWidth: 80
                            }

                            Label {
                                visible: hasAlertSelection
                                text: alertDetail.message ? I18n.t(alertDetail.message) : "-"
                                color: Theme.textPrimary
                                font.pixelSize: 16
                            }

                            FieldRow {
                                visible: hasAlertSelection
                                label: I18n.t("Device")
                                value: alertDetail.device ? alertDetail.device : "-"
                                labelWidth: 84
                            }

                            FieldRow {
                                visible: hasAlertSelection
                                label: I18n.t("Time")
                                value: alertDetail.time ? alertDetail.time : "-"
                                labelWidth: 84
                            }

                            FieldRow {
                                visible: hasAlertSelection
                                label: I18n.t("Status")
                                value: alertMuted ? I18n.t("Muted") : (alertAcknowledged ? I18n.t("Acknowledged") : I18n.t("Active"))
                                labelWidth: 84
                            }

                            TrendChart {
                                visible: hasAlertSelection
                                Layout.fillWidth: true
                                Layout.preferredHeight: 100
                                points: alertsVM.evidencePoints
                            }

                            FieldRow {
                                visible: hasAlertSelection
                                label: I18n.t("Evidence")
                                value: alertDetail.evidence ? I18n.t(alertDetail.evidence) : "-"
                                multiline: true
                                labelWidth: 84
                            }

                            FieldRow {
                                visible: hasAlertSelection
                                label: I18n.t("Suggested action")
                                value: alertDetail.action ? I18n.t(alertDetail.action) : "-"
                                multiline: true
                                labelWidth: 84
                            }

                            RowLayout {
                                visible: hasAlertSelection
                                Layout.fillWidth: true
                                spacing: 8

                                PrimaryButton {
                                    text: I18n.t("Acknowledge")
                                    Layout.fillWidth: true
                                    enabled: hasAlertSelection && !alertAcknowledged
                                    onClicked: {
                                        alertsVM.acknowledgeSelected()
                                        alertToast.text = I18n.t("Alert acknowledged (mock)")
                                        alertToast.open()
                                    }
                                }
                                GhostButton {
                                    text: I18n.t("Mute")
                                    Layout.fillWidth: true
                                    enabled: hasAlertSelection && !alertMuted
                                    onClicked: {
                                        alertsVM.muteSelected()
                                        alertToast.text = I18n.t("Alert muted (mock)")
                                        alertToast.open()
                                    }
                                }
                            }

                            Item { Layout.fillHeight: true }
                        }
                    }
                }
            }
        }
    }
}
