import QtQuick

GlassBackground {
    id: toggleButton

    width: 52
    height: 52
    cornerRadius: 26

    property bool menuOpen: false

    signal toggled()

    scale:
        toggleArea.pressed
        ? 0.92
        : 1.0

    Behavior on scale {
        NumberAnimation {
            duration: 90
        }
    }

    Text {
        anchors.centerIn: parent

        text:
            toggleButton.menuOpen
            ? "✕"
            : "☰"

        color: "white"

        font.pixelSize: 22
    }

    MouseArea {
        id: toggleArea

        anchors.fill: parent

        cursorShape: Qt.PointingHandCursor

        onClicked: toggleButton.toggled()
    }
}
