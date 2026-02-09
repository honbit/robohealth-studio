import QtQuick
import QtQuick.Controls
import "../theme"

Rectangle {
    id: root
    property string text: "Badge"
    property string tone: "accent" // accent|success|warn|muted

    radius: Theme.radiusSm
    height: 22
    color: tone === "success" ? Theme.successSoft
        : (tone === "warn" ? Theme.warnSoft
        : (tone === "muted" ? Theme.surfaceAlt : Theme.accentSoft))
    border.color: tone === "success" ? Theme.success
        : (tone === "warn" ? Theme.warn
        : (tone === "muted" ? Theme.border : Theme.accent))

    Label {
        anchors.centerIn: parent
        text: root.text
        font.pixelSize: 11
        color: tone === "success" ? Theme.success
            : (tone === "warn" ? Theme.warn
            : (tone === "muted" ? Theme.textMuted : Theme.accent))
    }
}
