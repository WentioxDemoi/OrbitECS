import QtQuick
import QtQuick.Effects

Item {
    id: gb

    property Item backdrop
    property real cornerRadius: 20

    default property alias content: contentItem.data

    ShaderEffectSource {
        id: src

        anchors.fill: parent
        sourceItem: gb.backdrop

        sourceRect: Qt.rect(gb.x, gb.y, gb.width, gb.height)

        visible: false
    }

    Item {
        id: maskItem

        anchors.fill: parent
        visible: false
        layer.enabled: true

        Rectangle {
            anchors.fill: parent
            radius: gb.cornerRadius
        }
    }

    MultiEffect {
        anchors.fill: parent
        source: src

        blurEnabled: true
        blur: 1.0
        blurMax: 48

        saturation: 0.4
        brightness: 0.08

        maskEnabled: true
        maskSource: maskItem
    }

    Rectangle {
        anchors.fill: parent
        radius: gb.cornerRadius

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "#40ffffff"
            }

            GradientStop {
                position: 0.5
                color: "#0dffffff"
            }

            GradientStop {
                position: 1.0
                color: "#22ffffff"
            }
        }

        border.width: 1
        border.color: "#66ffffff"
    }

    Item {
        id: contentItem
        anchors.fill: parent
    }
}
