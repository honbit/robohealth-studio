pragma Singleton
import QtQuick

QtObject {
    property string mode: "light" // light|dark

    readonly property color accent: "#FF8A00"
    readonly property color accentSoft: mode === "dark" ? "#523A1A" : "#FFF1E6"
    readonly property color success: mode === "dark" ? "#3DDC97" : "#2ECC71"
    readonly property color successSoft: mode === "dark" ? "#233C31" : "#E8F7EE"
    readonly property color warn: mode === "dark" ? "#F5B861" : "#F39C12"
    readonly property color warnSoft: mode === "dark" ? "#4A3520" : "#FFF4E5"
    readonly property color background: mode === "dark" ? "#0F1115" : "#F5F6F8"
    readonly property color surface: mode === "dark" ? "#1A202C" : "#FFFFFF"
    readonly property color surfaceAlt: mode === "dark" ? "#242B39" : "#FBFBFC"
    readonly property color textPrimary: mode === "dark" ? "#E6E8EB" : "#1F2328"
    readonly property color textMuted: mode === "dark" ? "#B0BAC9" : "#6B7280"
    readonly property color border: mode === "dark" ? "#3A4252" : "#E6E8EB"
    readonly property color danger: mode === "dark" ? "#FF6B6B" : "#E74C3C"

    readonly property int radiusSm: 6
    readonly property int radiusMd: 8
    readonly property int spacingSm: 8
    readonly property int spacingMd: 12
}
