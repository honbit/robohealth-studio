import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "../components"

Page {
    id: root
    title: I18n.t("Replay")

    function indexForTimeRange(value) {
        for (var i = 0; i < timeModel.count; i++) {
            if (timeModel.get(i).value === value) {
                return i
            }
        }
        return 1
    }

    function indexForSession(value) {
        for (var i = 0; i < sessionModel.count; i++) {
            if (sessionModel.get(i).value === value) {
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

    Component.onCompleted: {
        timeCombo.currentIndex = indexForTimeRange(settingsVM.timeRange)
        replayVM.timeRange = settingsVM.timeRange
        sessionCombo.currentIndex = indexForSession(replayVM.session)
        deviceCombo.currentIndex = indexForDevice(replayVM.device)
    }

    Connections {
        target: settingsVM
        function onTimeRangeChanged() {
            var nextIndex = indexForTimeRange(settingsVM.timeRange)
            if (timeCombo.currentIndex !== nextIndex) {
                timeCombo.currentIndex = nextIndex
            }
            if (replayVM.timeRange !== settingsVM.timeRange) {
                replayVM.timeRange = settingsVM.timeRange
            }
        }
    }

    Connections {
        target: replayVM
        function onSessionChanged() {
            var nextIndex = indexForSession(replayVM.session)
            if (sessionCombo.currentIndex !== nextIndex) {
                sessionCombo.currentIndex = nextIndex
            }
        }
        function onDeviceChanged() {
            var nextIndex = indexForDevice(replayVM.device)
            if (deviceCombo.currentIndex !== nextIndex) {
                deviceCombo.currentIndex = nextIndex
            }
        }
        function onPlaybackRateChanged() {
            if (replayVM.playbackRate >= 4) {
                speedCombo.currentIndex = 2
            } else if (replayVM.playbackRate >= 2) {
                speedCombo.currentIndex = 1
            } else {
                speedCombo.currentIndex = 0
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

                SectionHeader { title: I18n.t("Replay"); subtitle: I18n.t("Timeline and playback") }

                ListModel {
                    id: timeModel
                    ListElement { label: "Last 1h"; value: "1h" }
                    ListElement { label: "Last 24h"; value: "24h" }
                    ListElement { label: "Last 7d"; value: "7d" }
                }

                ListModel {
                    id: sessionModel
                    ListElement { label: "Session A"; value: "Session A" }
                    ListElement { label: "Session B"; value: "Session B" }
                    ListElement { label: "Session C"; value: "Session C" }
                }

                ListModel {
                    id: deviceModel
                    ListElement { label: "Robot-01"; value: "Robot-01" }
                    ListElement { label: "Robot-02"; value: "Robot-02" }
                    ListElement { label: "Cam-04"; value: "Cam-04" }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ComboBox {
                        id: sessionCombo
                        Layout.preferredWidth: 160
                        model: sessionModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                var value = sessionModel.get(currentIndex).value
                                if (replayVM.session !== value) {
                                    replayVM.session = value
                                }
                                replayVM.loadSession()
                            }
                        }
                    }
                    ComboBox {
                        id: deviceCombo
                        Layout.preferredWidth: 140
                        model: deviceModel
                        textRole: "label"
                        displayText: I18n.t(currentText)
                        delegate: ItemDelegate { text: I18n.t(label) }
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                var value = deviceModel.get(currentIndex).value
                                if (replayVM.device !== value) {
                                    replayVM.device = value
                                }
                                replayVM.loadSession()
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
                                if (settingsVM.timeRange !== value) {
                                    settingsVM.timeRange = value
                                }
                                if (replayVM.timeRange !== value) {
                                    replayVM.timeRange = value
                                }
                                replayVM.loadSession()
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    GhostButton { text: I18n.t("Load"); onClicked: replayVM.loadSession() }
                    PrimaryButton { text: I18n.t("Start"); onClicked: replayVM.start() }
                }

                Card {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        Label { text: I18n.t("Timeline"); color: Theme.textMuted; font.pixelSize: 12 }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 24
                            radius: Theme.radiusSm
                            color: Theme.surfaceAlt
                            border.color: Theme.border

                            Rectangle {
                                width: parent.width * replayVM.progress
                                height: parent.height
                                radius: Theme.radiusSm
                                color: Theme.accentSoft
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            GhostButton { text: I18n.t("Play"); enabled: !replayVM.playing; onClicked: replayVM.play() }
                            GhostButton { text: I18n.t("Pause"); enabled: replayVM.playing; onClicked: replayVM.pause() }
                            GhostButton { text: I18n.t("Stop"); enabled: replayVM.playing || replayVM.progress > 0; onClicked: replayVM.stop() }
                            ComboBox {
                                id: speedCombo
                                Layout.preferredWidth: 120
                                model: ["1x", "2x", "4x"]
                                currentIndex: 0
                                onCurrentIndexChanged: {
                                    if (currentIndex === 1) {
                                        replayVM.playbackRate = 2
                                    } else if (currentIndex === 2) {
                                        replayVM.playbackRate = 4
                                    } else {
                                        replayVM.playbackRate = 1
                                    }
                                }
                            }

                            Item { Layout.fillWidth: true }

                            Label { text: replayVM.timeLabel; color: Theme.textMuted }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    TrendChart { Layout.fillWidth: true; Layout.preferredHeight: 260; points: replayVM.trendPoints }
                    Card {
                        Layout.preferredWidth: 320
                        Layout.preferredHeight: 260

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8

                            Label { text: I18n.t("Events"); color: Theme.textMuted; font.pixelSize: 12 }

                            ListView {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                model: replayVM.events
                                spacing: 6

                                delegate: Rectangle {
                                    width: ListView.view ? ListView.view.width : 0
                                    height: 32
                                    radius: Theme.radiusSm
                                    color: Theme.surfaceAlt
                                    border.color: Theme.border

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 6
                                        spacing: 8

                                        Label { text: time; color: Theme.textMuted; Layout.preferredWidth: 50 }
                                        Label { text: I18n.t(text); color: Theme.textPrimary; Layout.fillWidth: true }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
