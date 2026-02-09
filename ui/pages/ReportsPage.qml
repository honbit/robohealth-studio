import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "../components"

Page {
    id: root
    title: I18n.t("Reports")

    property var reportDetail: reportsVM.selectedReport
    property bool listLoading: true
    property bool hasReportSelection: reportsVM.selectedIndex >= 0 && reportDetail.title

    function indexForTimeRange(value) {
        for (var i = 0; i < timeModel.count; i++) {
            if (timeModel.get(i).value === value) {
                return i
            }
        }
        return 1
    }

    function indexForReportType(value) {
        for (var i = 0; i < reportTypeModel.count; i++) {
            if (reportTypeModel.get(i).value === value) {
                return i
            }
        }
        return 0
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

    function reportFilterSummary() {
        var parts = []
        if (typeCombo.currentIndex > 0) {
            parts.push(I18n.t("Report type") + ": " + I18n.t(typeCombo.currentText))
        }
        if (timeCombo.currentIndex > 0) {
            parts.push(I18n.t("Time") + ": " + I18n.t(timeCombo.currentText))
        }
        if (deviceCombo.currentIndex > 0) {
            parts.push(I18n.t("Device") + ": " + I18n.t(deviceCombo.currentText))
        }
        return parts.join(" · ")
    }

    function resetFilters() {
        typeCombo.currentIndex = 0
        timeCombo.currentIndex = indexForTimeRange("24h")
        deviceCombo.currentIndex = 0
        settingsVM.timeRange = "24h"
        triggerLoading()
    }

    Component.onCompleted: {
        timeCombo.currentIndex = indexForTimeRange(settingsVM.timeRange)
        reportsVM.timeRange = settingsVM.timeRange
        typeCombo.currentIndex = indexForReportType(reportsVM.reportType)
        deviceCombo.currentIndex = indexForDevice(reportsVM.deviceFilter)
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
        target: reportsVM.reports
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
            if (reportsVM.timeRange !== settingsVM.timeRange) {
                reportsVM.timeRange = settingsVM.timeRange
                triggerLoading()
            }
        }
    }

    Connections {
        target: reportsVM
        function onReportTypeChanged() {
            var nextIndex = indexForReportType(reportsVM.reportType)
            if (typeCombo.currentIndex !== nextIndex) {
                typeCombo.currentIndex = nextIndex
            }
        }
        function onDeviceFilterChanged() {
            var nextIndex = indexForDevice(reportsVM.deviceFilter)
            if (deviceCombo.currentIndex !== nextIndex) {
                deviceCombo.currentIndex = nextIndex
            }
        }
    }

    Snackbar {
        id: reportToast
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

                SectionHeader { title: I18n.t("Reports"); subtitle: I18n.t("Summary and export") }

                ListModel {
                    id: timeModel
                    ListElement { label: "Last 1h"; value: "1h" }
                    ListElement { label: "Last 24h"; value: "24h" }
                    ListElement { label: "Last 7d"; value: "7d" }
                }

                ListModel {
                    id: reportTypeModel
                    ListElement { label: "Daily"; value: "Daily" }
                    ListElement { label: "Weekly"; value: "Weekly" }
                    ListElement { label: "Monthly"; value: "Monthly" }
                }

                ListModel {
                    id: deviceModel
                    ListElement { label: "All Devices"; value: "" }
                    ListElement { label: "Robot-01"; value: "Robot-01" }
                    ListElement { label: "Robot-02"; value: "Robot-02" }
                    ListElement { label: "Cam-04"; value: "Cam-04" }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ComboBox {
                        id: typeCombo
                        Layout.preferredWidth: 160
                        Layout.preferredHeight: 32
                        font.pixelSize: 12
                        model: reportTypeModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                var value = reportTypeModel.get(currentIndex).value
                                if (reportsVM.reportType !== value) {
                                    reportsVM.reportType = value
                                }
                                reportsVM.preview()
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
                                if (settingsVM.timeRange !== value) {
                                    settingsVM.timeRange = value
                                }
                                if (reportsVM.timeRange !== value) {
                                    reportsVM.timeRange = value
                                }
                                reportsVM.preview()
                                triggerLoading()
                            }
                        }
                    }
                    ComboBox {
                        id: deviceCombo
                        Layout.preferredWidth: 160
                        Layout.preferredHeight: 32
                        font.pixelSize: 12
                        model: deviceModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                var value = deviceModel.get(currentIndex).value
                                if (reportsVM.deviceFilter !== value) {
                                    reportsVM.deviceFilter = value
                                }
                                reportsVM.preview()
                                triggerLoading()
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    GhostButton {
                        text: I18n.t("Reset")
                        onClicked: resetFilters()
                    }
                    GhostButton { text: I18n.t("Preview"); onClicked: reportsVM.preview() }
                    PrimaryButton {
                        text: I18n.t("Export")
                        enabled: hasReportSelection
                        onClicked: {
                            var path = reportsVM.exportReport()
                            if (path) {
                                reportToast.text = I18n.t("Exported to: ") + path + " " + I18n.t("(mock)")
                            } else {
                                reportToast.text = I18n.t("Export failed")
                            }
                            reportToast.open()
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    visible: reportFilterSummary().length > 0
                    text: I18n.t("Filters: ") + reportFilterSummary()
                    color: Theme.textMuted
                    font.pixelSize: 12
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

                            Label { text: I18n.t("Report List"); color: Theme.textMuted; font.pixelSize: 12 }

                            TableHeader {
                                Layout.fillWidth: true
                                HeaderLabel { text: I18n.t("Report"); Layout.fillWidth: true }
                                HeaderLabel { text: I18n.t("Time"); Layout.preferredWidth: 90 }
                            }

                            ListView {
                                id: reportList
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                model: reportsVM.reports
                                spacing: 6
                                visible: !listLoading && count > 0
                                clip: true
                                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                                delegate: DataRow {
                                    width: ListView.view ? ListView.view.width : 0
                                    selected: ListView.isCurrentItem

                                    onClicked: {
                                        reportsVM.selectedIndex = index
                                    }

                                    Label { text: I18n.t(title); color: Theme.textPrimary; Layout.fillWidth: true }
                                    Label { text: time; color: Theme.textMuted; Layout.preferredWidth: 90 }
                                }

                                currentIndex: reportsVM.selectedIndex
                            }

                            SkeletonTable {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: listLoading
                                rows: 5
                                columnRatios: [0.7, 0.3]
                            }

                            EmptyState {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: !listLoading && reportList.count === 0
                                title: reportFilterSummary().length > 0 ? I18n.t("No results") : I18n.t("No data")
                                subtitle: reportFilterSummary().length > 0 ? I18n.t("Try clearing filters") : ""
                                actionText: reportFilterSummary().length > 0 ? I18n.t("Clear filters") : ""
                                onActionTriggered: resetFilters()
                                compact: true
                            }
                        }
                    }

                    Card {
                        Layout.preferredWidth: 360
                        Layout.preferredHeight: 420

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10

                            Label { text: I18n.t("Preview"); color: Theme.textMuted; font.pixelSize: 12 }

                            EmptyState {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: !hasReportSelection
                                title: I18n.t("Select a report to preview")
                                subtitle: I18n.t("Select from the list")
                                compact: true
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                spacing: 8
                                visible: hasReportSelection

                                Label { text: I18n.t(reportDetail.title); color: Theme.textPrimary; font.pixelSize: 16 }
                                Label { text: reportDetail.time; color: Theme.textMuted; font.pixelSize: 12 }
                                Label {
                                    text: I18n.t(reportDetail.summary)
                                    color: Theme.textMuted
                                    wrapMode: Text.Wrap
                                }

                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    rowSpacing: 8
                                    columnSpacing: 8

                                    Repeater {
                                        model: reportsVM.previewMetrics
                                        delegate: KpiCard {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 72
                                            label: modelData.label
                                            value: modelData.value
                                            unit: modelData.unit
                                            alerted: modelData.alerted
                                        }
                                    }
                                }

                                Label { text: I18n.t("Highlights"); color: Theme.textMuted; font.pixelSize: 12 }

                                Repeater {
                                    model: reportsVM.previewHighlights
                                    delegate: RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 6
                                        Label { text: "•"; color: Theme.textMuted }
                                        Label {
                                            text: I18n.t(modelData)
                                            color: Theme.textPrimary
                                            wrapMode: Text.Wrap
                                            Layout.fillWidth: true
                                        }
                                    }
                                }

                                Item { Layout.fillHeight: true }
                            }

                            PrimaryButton {
                                text: I18n.t("Export PDF")
                                Layout.fillWidth: true
                                enabled: hasReportSelection
                                onClicked: {
                                    var path = reportsVM.exportPdf()
                                    if (path) {
                                        reportToast.text = I18n.t("Exported to: ") + path + " " + I18n.t("(mock)")
                                    } else {
                                        reportToast.text = I18n.t("Export failed")
                                    }
                                    reportToast.open()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
