import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "../components"

Page {
    id: root
    title: I18n.t("Devices")

    property var deviceDetail: devicesVM.selectedDevice
    property bool listLoading: true
    property bool hasDeviceSelection: devicesVM.selectedIndex >= 0 && deviceDetail.name

    function triggerLoading() {
        listLoading = true
        loadingTimer.restart()
    }

    function stopLoading() {
        listLoading = false
        loadingTimer.stop()
    }

    function deviceFilterSummary() {
        var parts = []
        if (searchField.text.trim().length > 0) {
            parts.push(I18n.t("Keyword") + ": " + searchField.text.trim())
        }
        if (typeCombo.currentIndex > 0) {
            parts.push(I18n.t("Type") + ": " + I18n.t(typeCombo.currentText))
        }
        if (statusCombo.currentIndex > 0) {
            parts.push(I18n.t("Status") + ": " + I18n.t(statusCombo.currentText))
        }
        return parts.join(" · ")
    }

    function resetFilters() {
        searchField.text = ""
        typeCombo.currentIndex = 0
        statusCombo.currentIndex = 0
        triggerLoading()
    }

    Component.onCompleted: {
        devicesVM.filteredDevices.searchText = searchField.text
        if (typeCombo.currentIndex >= 0) {
            devicesVM.filteredDevices.typeFilter = typeModel.get(typeCombo.currentIndex).value
        }
        if (statusCombo.currentIndex >= 0) {
            devicesVM.filteredDevices.statusFilter = statusModel.get(statusCombo.currentIndex).value
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
        target: devicesVM.filteredDevices
        function onModelReset() { stopLoading() }
        function onRowsInserted() { stopLoading() }
        function onRowsRemoved() { stopLoading() }
        function onLayoutChanged() { stopLoading() }
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
                    title: I18n.t("Devices")
                    subtitle: I18n.t("Inventory and health")
                }

                ListModel {
                    id: typeModel
                    ListElement { label: "All Types"; value: "" }
                    ListElement { label: "Robot"; value: "Robot" }
                    ListElement { label: "Camera"; value: "Camera" }
                    ListElement { label: "IMU"; value: "IMU" }
                    ListElement { label: "PLC"; value: "PLC" }
                }

                ListModel {
                    id: statusModel
                    ListElement { label: "All Status"; value: "" }
                    ListElement { label: "Online"; value: "Online" }
                    ListElement { label: "Offline"; value: "Offline" }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    TextField {
                        id: searchField
                        Layout.preferredWidth: 240
                        Layout.preferredHeight: 32
                        font.pixelSize: 12
                        placeholderText: I18n.t("Search by name or id")
                        onTextChanged: devicesVM.filteredDevices.searchText = text
                        onEditingFinished: triggerLoading()
                    }

                    ComboBox {
                        id: typeCombo
                        Layout.preferredWidth: 140
                        Layout.preferredHeight: 32
                        font.pixelSize: 12
                        model: typeModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                devicesVM.filteredDevices.typeFilter = typeModel.get(currentIndex).value
                                triggerLoading()
                            }
                        }
                    }

                    ComboBox {
                        id: statusCombo
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 32
                        font.pixelSize: 12
                        model: statusModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                devicesVM.filteredDevices.statusFilter = statusModel.get(currentIndex).value
                                triggerLoading()
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    GhostButton {
                        text: I18n.t("Reset")
                        onClicked: resetFilters()
                    }
                    PrimaryButton { text: I18n.t("Connect all"); onClicked: devicesVM.connectAll() }
                }

                Label {
                    Layout.fillWidth: true
                    visible: deviceFilterSummary().length > 0
                    text: I18n.t("Filters: ") + deviceFilterSummary()
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
                                HeaderLabel { text: I18n.t("Name"); Layout.fillWidth: true }
                                HeaderLabel { text: I18n.t("Type"); Layout.preferredWidth: 80 }
                                HeaderLabel { text: I18n.t("Status"); Layout.preferredWidth: 80 }
                                HeaderLabel { text: I18n.t("Last seen"); Layout.preferredWidth: 80 }
                                HeaderLabel { text: I18n.t("Health"); Layout.preferredWidth: 60 }
                            }

                            ListView {
                                id: deviceList
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                model: devicesVM.filteredDevices
                                spacing: 4
                                currentIndex: devicesVM.selectedIndex
                                visible: !listLoading && count > 0
                                clip: true
                                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                                delegate: DataRow {
                                    width: ListView.view ? ListView.view.width : 0
                                    selected: ListView.isCurrentItem

                                    onClicked: {
                                        deviceList.currentIndex = index
                                        devicesVM.selectedIndex = index
                                    }

                                    Label { text: name; color: Theme.textPrimary; Layout.fillWidth: true }
                                    Label { text: I18n.t(type); color: Theme.textMuted; Layout.preferredWidth: 80 }
                                    Badge {
                                        text: I18n.t(status)
                                        tone: status === "Online" ? "success" : "muted"
                                        Layout.preferredWidth: 72
                                    }
                                    Label { text: lastSeen; color: Theme.textMuted; Layout.preferredWidth: 80 }
                                    Label { text: health + "%"; color: Theme.textPrimary; Layout.preferredWidth: 60 }
                                }
                            }

                            SkeletonTable {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: listLoading
                                rows: 6
                                columnRatios: [0.36, 0.16, 0.16, 0.2, 0.12]
                            }

                            EmptyState {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                visible: !listLoading && deviceList.count === 0
                                title: deviceFilterSummary().length > 0 ? I18n.t("No results") : I18n.t("No data")
                                subtitle: deviceFilterSummary().length > 0 ? I18n.t("Try clearing filters") : ""
                                actionText: deviceFilterSummary().length > 0 ? I18n.t("Clear filters") : ""
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

                            Label { text: I18n.t("Device Detail"); color: Theme.textPrimary; font.pixelSize: 14 }

                            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

                            EmptyState {
                                Layout.fillWidth: true
                                visible: !hasDeviceSelection
                                title: I18n.t("Select a device to view details")
                                subtitle: I18n.t("Select from the list")
                            }

                            Label {
                                visible: hasDeviceSelection
                                text: deviceDetail.name ? deviceDetail.name : "-"
                                color: Theme.textPrimary
                                font.pixelSize: 18
                            }

                            Badge {
                                visible: hasDeviceSelection
                                text: deviceDetail.status ? I18n.t(deviceDetail.status) : "-"
                                tone: deviceDetail.status === "Online" ? "success" : "muted"
                                Layout.preferredWidth: 80
                            }

                            FieldRow {
                                visible: hasDeviceSelection
                                label: I18n.t("Health score")
                                value: deviceDetail.health !== undefined ? (deviceDetail.health + "%") : "-"
                                labelWidth: 80
                            }

                            FieldRow {
                                visible: hasDeviceSelection
                                label: I18n.t("Last seen")
                                value: deviceDetail.lastSeen ? deviceDetail.lastSeen : "-"
                                labelWidth: 80
                            }

                            Rectangle {
                                visible: hasDeviceSelection
                                Layout.fillWidth: true
                                Layout.preferredHeight: 120
                                radius: Theme.radiusSm
                                color: Theme.surfaceAlt
                                border.color: Theme.border

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 6

                                    Label { text: I18n.t("Mini chart"); color: Theme.textMuted; font.pixelSize: 12 }
                                    SparkLine {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        points: devicesVM.deviceTrendPoints
                                        alerted: deviceDetail.status === "Offline"
                                            || (deviceDetail.health !== undefined && deviceDetail.health < 70)
                                    }
                                }
                            }

                            RowLayout {
                                visible: hasDeviceSelection
                                Layout.fillWidth: true
                                spacing: 8

                                PrimaryButton {
                                    text: I18n.t("Simulate fault")
                                    Layout.fillWidth: true
                                    enabled: hasDeviceSelection
                                    onClicked: devicesVM.simulateFault()
                                }
                                GhostButton {
                                    text: I18n.t("Disconnect")
                                    Layout.fillWidth: true
                                    enabled: hasDeviceSelection
                                    onClicked: devicesVM.disconnectSelected()
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
