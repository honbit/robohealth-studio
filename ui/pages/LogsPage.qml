import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "../components"

Page {
    id: root
    title: I18n.t("Logs")

    property var logDetail: logsVM.selectedLog
    property bool listLoading: true
    property bool hasLogSelection: !!(logsVM.selectedIndex >= 0 && logDetail && logDetail.message)

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

    function logFilterSummary() {
        var parts = []
        if (searchField.text.trim().length > 0) {
            parts.push(I18n.t("Keyword") + ": " + searchField.text.trim())
        }
        if (levelCombo.currentIndex > 0) {
            parts.push(I18n.t("Level") + ": " + I18n.t(levelCombo.currentText))
        }
        if (timeCombo.currentIndex > 0) {
            parts.push(I18n.t("Time") + ": " + I18n.t(timeCombo.currentText))
        }
        return parts.join(" · ")
    }

    function resetFilters() {
        searchField.text = ""
        levelCombo.currentIndex = 0
        timeCombo.currentIndex = 1
        triggerLoading()
    }

    Component.onCompleted: {
        if (levelCombo.currentIndex >= 0) {
            logsVM.filteredLogs.levelFilter = levelModel.get(levelCombo.currentIndex).value
        }
        timeCombo.currentIndex = indexForTimeRange(settingsVM.timeRange)
        if (timeCombo.currentIndex >= 0) {
            logsVM.filteredLogs.timeRange = timeModel.get(timeCombo.currentIndex).value
        }
        logsVM.filteredLogs.searchText = searchField.text
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
        target: logsVM.filteredLogs
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
        id: logToast
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

                SectionHeader { title: I18n.t("Logs"); subtitle: I18n.t("Search and inspect") }

                ListModel {
                    id: levelModel
                    ListElement { label: "All Levels"; value: "" }
                    ListElement { label: "Error"; value: "Error" }
                    ListElement { label: "Warn"; value: "Warn" }
                    ListElement { label: "Info"; value: "Info" }
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

                    TextField {
                        id: searchField
                        Layout.preferredWidth: 260
                        Layout.preferredHeight: 32
                        font.pixelSize: 12
                        placeholderText: I18n.t("Search keyword")
                        onTextChanged: logsVM.filteredLogs.searchText = text
                        onEditingFinished: triggerLoading()
                    }
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
                                logsVM.filteredLogs.levelFilter = levelModel.get(currentIndex).value
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
                                logsVM.filteredLogs.timeRange = value
                                if (settingsVM.timeRange !== value) {
                                    settingsVM.timeRange = value
                                }
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
                            var path = logsVM.exportLogs()
                            if (path) {
                                logToast.text = I18n.t("Exported to: ") + path + " " + I18n.t("(mock)")
                            } else {
                                logToast.text = I18n.t("Export failed")
                            }
                            logToast.open()
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: logFilterSummary().length > 0
                    text: I18n.t("Filters: ") + logFilterSummary()
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
                                HeaderLabel { text: I18n.t("Level"); Layout.preferredWidth: 70 }
                                HeaderLabel { text: I18n.t("Message"); Layout.fillWidth: true }
                                HeaderLabel { text: I18n.t("Device"); Layout.preferredWidth: 90 }
                                HeaderLabel { text: I18n.t("Time"); Layout.preferredWidth: 70 }
                            }

                            ListView {
                                id: logList
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                model: logsVM.filteredLogs
                                spacing: 4
                                currentIndex: logsVM.selectedIndex
                                visible: !listLoading && count > 0
                                clip: true
                                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                                delegate: DataRow {
                                    width: ListView.view ? ListView.view.width : 0
                                    selected: ListView.isCurrentItem

                                    onClicked: {
                                        logList.currentIndex = index
                                        logsVM.selectedIndex = index
                                    }

                                    Badge {
                                        text: I18n.t(level)
                                        tone: level === "Error" ? "warn" : (level === "Warn" ? "accent" : "muted")
                                        Layout.preferredWidth: 64
                                    }
                                    Label { text: I18n.t(message); color: Theme.textPrimary; Layout.fillWidth: true }
                                    Label { text: device; color: Theme.textMuted; Layout.preferredWidth: 90 }
                                    Label { text: time; color: Theme.textMuted; Layout.preferredWidth: 70 }
                                }
                            }

                            SkeletonTable {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: listLoading
                                rows: 6
                                columnRatios: [0.18, 0.5, 0.2, 0.12]
                            }

                            EmptyState {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: !listLoading && logList.count === 0
                                title: logFilterSummary().length > 0 ? I18n.t("No results") : I18n.t("No data")
                                subtitle: logFilterSummary().length > 0 ? I18n.t("Try clearing filters") : ""
                                actionText: logFilterSummary().length > 0 ? I18n.t("Clear filters") : ""
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

                            Label { text: I18n.t("Log Detail"); color: Theme.textPrimary; font.pixelSize: 14 }
                            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

                            EmptyState {
                                Layout.fillWidth: true
                                visible: !hasLogSelection
                                title: I18n.t("Select a log to view details")
                                subtitle: I18n.t("Select from the list")
                            }

                            Label {
                                visible: hasLogSelection
                                text: logDetail.message ? I18n.t(logDetail.message) : "-"
                                color: Theme.textPrimary
                                font.pixelSize: 16
                                wrapMode: Text.Wrap
                            }

                            FieldRow {
                                visible: hasLogSelection
                                label: I18n.t("Device")
                                value: logDetail.device ? logDetail.device : "-"
                                labelWidth: 72
                            }

                            FieldRow {
                                visible: hasLogSelection
                                label: I18n.t("Time")
                                value: logDetail.time ? logDetail.time : "-"
                                labelWidth: 72
                            }

                            Rectangle {
                                visible: hasLogSelection
                                Layout.fillWidth: true
                                Layout.preferredHeight: 160
                                radius: Theme.radiusSm
                                color: Theme.surfaceAlt
                                border.color: Theme.border

                                ListView {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    clip: true
                                    spacing: 6
                                    model: logsVM.contextLines

                                    delegate: DataRow {
                                        width: ListView.view ? ListView.view.width : 0
                                        height: 32
                                        hoverable: false

                                        Label {
                                            text: modelData && modelData.time ? modelData.time : "-"
                                            color: Theme.textMuted
                                            Layout.preferredWidth: 50
                                        }
                                        Label {
                                            text: modelData && modelData.text ? I18n.t(modelData.text) : "-"
                                            color: Theme.textPrimary
                                            Layout.fillWidth: true
                                        }
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
