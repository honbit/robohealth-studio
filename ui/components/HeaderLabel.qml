import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"

Label {
    font.pixelSize: 12
    font.weight: Font.DemiBold
    color: Theme.textMuted
    elide: Text.ElideRight
    horizontalAlignment: Text.AlignLeft
    verticalAlignment: Text.AlignVCenter
    Layout.alignment: Qt.AlignVCenter
}
