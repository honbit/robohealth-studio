import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../state"
import "../components"

Page {
    id: root
    title: I18n.t("Settings")

    Component.onCompleted: {
        settingsVM.language = I18n.language
        settingsVM.theme = Theme.mode
    }

    Connections {
        target: settingsVM
        function onLanguageChanged() {
            if (I18n.language !== settingsVM.language) {
                I18n.language = settingsVM.language
            }
        }
        function onThemeChanged() {
            if (Theme.mode !== settingsVM.theme) {
                Theme.mode = settingsVM.theme
            }
            if (themeCombo && themeCombo.currentIndex !== (settingsVM.theme === "dark" ? 1 : 0)) {
                themeCombo.currentIndex = settingsVM.theme === "dark" ? 1 : 0
            }
        }
    }

    Connections {
        target: I18n
        function onLanguageChanged() {
            if (settingsVM.language !== I18n.language) {
                settingsVM.language = I18n.language
            }
        }
    }

    Connections {
        target: Theme
        function onModeChanged() {
            if (settingsVM.theme !== Theme.mode) {
                settingsVM.theme = Theme.mode
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

                ListModel {
                    id: themeModel
                    ListElement { label: "Light"; value: "light" }
                    ListElement { label: "Dark"; value: "dark" }
                }

                SectionHeader { title: I18n.t("Settings"); subtitle: I18n.t("Language and simulator") }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Card {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 260

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10

                            Label { text: I18n.t("General"); color: Theme.textMuted; font.pixelSize: 12 }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: I18n.t("Language"); color: Theme.textPrimary; Layout.preferredWidth: 120 }
                                LanguageToggle {
                                    Layout.preferredWidth: 90
                                    currentLanguage: settingsVM.language
                                    onLanguageSelected: function(value) { settingsVM.language = value }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: I18n.t("Theme"); color: Theme.textPrimary; Layout.preferredWidth: 120 }
                                ComboBox {
                                    id: themeCombo
                                    model: themeModel
                                    textRole: "label"
                                    displayText: I18n.t(currentText)
                                    delegate: ItemDelegate { text: I18n.t(label) }
                                    Layout.preferredWidth: 140
                                    onCurrentIndexChanged: {
                                        if (currentIndex >= 0) {
                                            settingsVM.theme = themeModel.get(currentIndex).value
                                        }
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: I18n.t("Auto refresh"); color: Theme.textPrimary; Layout.preferredWidth: 120 }
                                Switch {
                                    checked: settingsVM.autoRefresh
                                    onToggled: settingsVM.autoRefresh = checked
                                }
                            }

                            Item { Layout.fillHeight: true }
                        }
                    }

                    Card {
                        Layout.preferredWidth: 320
                        Layout.preferredHeight: 260

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10

                            Label { text: I18n.t("Simulator"); color: Theme.textMuted; font.pixelSize: 12 }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: I18n.t("Device count"); color: Theme.textPrimary; Layout.preferredWidth: 120 }
                                SpinBox {
                                    value: settingsVM.deviceCount
                                    from: 1
                                    to: 200
                                    Layout.preferredWidth: 120
                                    onValueModified: settingsVM.deviceCount = value
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: I18n.t("Fault rate"); color: Theme.textPrimary; Layout.preferredWidth: 120 }
                                Slider {
                                    value: settingsVM.faultRate
                                    from: 0
                                    to: 1
                                    Layout.preferredWidth: 120
                                    onMoved: settingsVM.faultRate = value
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: I18n.t("Refresh Hz"); color: Theme.textPrimary; Layout.preferredWidth: 120 }
                                SpinBox {
                                    value: settingsVM.refreshHz
                                    from: 1
                                    to: 10
                                    Layout.preferredWidth: 120
                                    onValueModified: settingsVM.refreshHz = value
                                }
                            }

                            Item { Layout.fillHeight: true }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Card {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 180

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10

                            Label { text: I18n.t("Thresholds"); color: Theme.textMuted; font.pixelSize: 12 }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: I18n.t("Vibration"); color: Theme.textPrimary; Layout.preferredWidth: 120 }
                                TextField {
                                    text: settingsVM.vibrationThreshold.toFixed(2)
                                    Layout.preferredWidth: 120
                                    validator: DoubleValidator { bottom: 0; top: 1000; decimals: 2 }
                                    onEditingFinished: {
                                        var value = Number(text)
                                        if (!isNaN(value)) {
                                            settingsVM.vibrationThreshold = value
                                        } else {
                                            text = settingsVM.vibrationThreshold.toFixed(2)
                                        }
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: I18n.t("Torque"); color: Theme.textPrimary; Layout.preferredWidth: 120 }
                                TextField {
                                    text: settingsVM.torqueThreshold.toFixed(1)
                                    Layout.preferredWidth: 120
                                    validator: DoubleValidator { bottom: 0; top: 1000; decimals: 2 }
                                    onEditingFinished: {
                                        var value = Number(text)
                                        if (!isNaN(value)) {
                                            settingsVM.torqueThreshold = value
                                        } else {
                                            text = settingsVM.torqueThreshold.toFixed(1)
                                        }
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: I18n.t("Current"); color: Theme.textPrimary; Layout.preferredWidth: 120 }
                                TextField {
                                    text: settingsVM.currentThreshold.toFixed(1)
                                    Layout.preferredWidth: 120
                                    validator: DoubleValidator { bottom: 0; top: 1000; decimals: 2 }
                                    onEditingFinished: {
                                        var value = Number(text)
                                        if (!isNaN(value)) {
                                            settingsVM.currentThreshold = value
                                        } else {
                                            text = settingsVM.currentThreshold.toFixed(1)
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
