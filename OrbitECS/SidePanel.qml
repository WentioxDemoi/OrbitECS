import QtQuick

GlassBackground {
    id: menuPanel

    cornerRadius: 24

    property int followedIndex: -1
    property alias contentHeight: menuList.contentHeight

    signal bodySelected(int index)
    signal freeViewRequested()

    ListView {
        id: menuList

        anchors.fill: parent
        anchors.margins: 8

        clip: true
        spacing: 2

        model: frontManager.heavyCount

        boundsBehavior: Flickable.StopAtBounds

        header: Item {
            width: menuList.width
            height: 40

            readonly property bool isFree: menuPanel.followedIndex < 0

            Rectangle {
                anchors.fill: parent
                anchors.margins: 3

                radius: height / 2

                gradient: Gradient {
                    GradientStop {
                        position: 0.0

                        color:
                            parent.parent.isFree
                            ? "#5580d0ff"
                            : "#33ffffff"
                    }

                    GradientStop {
                        position: 1.0

                        color:
                            parent.parent.isFree
                            ? "#2280d0ff"
                            : "#0fffffff"
                    }
                }

                border.width: 1

                border.color:
                    parent.isFree
                    ? "#aa80d0ff"
                    : "#55ffffff"

                Text {
                    anchors.centerIn: parent

                    text: "Vue libre"

                    color: "white"

                    font.pixelSize: 14
                    font.italic: true
                }

                MouseArea {
                    anchors.fill: parent

                    cursorShape: Qt.PointingHandCursor

                    onClicked: menuPanel.freeViewRequested()
                }
            }
        }

        delegate: Item {
            id: entry

            width: menuList.width
            height: 40

            readonly property bool isFollowed: index === menuPanel.followedIndex

            Rectangle {
                anchors.fill: parent
                anchors.margins: 3

                radius: height / 2

                gradient: Gradient {
                    GradientStop {
                        position: 0.0

                        color:
                            entry.isFollowed
                            ? "#66ffd54a"
                            : (
                                area.pressed
                                ? "#66ffffff"
                                : (
                                    area.containsMouse
                                    ? "#44ffffff"
                                    : "#33ffffff"
                                )
                            )
                    }

                    GradientStop {
                        position: 1.0

                        color:
                            entry.isFollowed
                            ? "#22ffd54a"
                            : "#0fffffff"
                    }
                }

                border.width: 1

                border.color:
                    entry.isFollowed
                    ? "#aaffd54a"
                    : "#55ffffff"

                Text {
                    anchors.centerIn: parent

                    text: frontManager.heavyName(index)

                    color: "white"

                    font.pixelSize: 14
                    font.bold: entry.isFollowed
                }

                MouseArea {
                    id: area

                    anchors.fill: parent

                    hoverEnabled: true

                    cursorShape: Qt.PointingHandCursor

                    onClicked: menuPanel.bodySelected(index)
                }
            }
        }
    }
}
