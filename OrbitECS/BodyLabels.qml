import QtQuick
import QtQuick3D

Repeater {
    id: labels

    property Camera camera
    property Item sceneView
    property int followedIndex: -1
    property int frameTick: 0

    model: frontManager.heavyCount

    delegate: Text {
        id: bodyName

        property vector3d viewportPosition: {
            labels.frameTick

            return labels.camera.mapToViewport(
                frontManager.heavyPosition(index)
            )
        }

        x: viewportPosition.x * labels.sceneView.width - width / 2
        y: viewportPosition.y * labels.sceneView.height - height - 8

        text: frontManager.heavyName(index)

        color: index === labels.followedIndex
               ? "#ffd54a"
               : "white"

        font.pixelSize: 13
        font.bold: true

        style: Text.Outline
        styleColor: "black"

        visible:
            viewportPosition.z >= 0
            && viewportPosition.x >= 0
            && viewportPosition.x <= 1
            && viewportPosition.y >= 0
            && viewportPosition.y <= 1
    }
}
