import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

// Encapsule le pivot de caméra et le suivi d'astre.
//
// originNode (root) est uniquement le pivot de la caméra.
// Les Model/instances restent dans la scène et ne sont jamais déplacés.

Node {
    id: root

    property alias camera: camera

    property int followedIndex: -1
    property real followDistance: 0.5
    property vector3d followOffset: Qt.vector3d(0, 0, 0.5)
    property int frameTick: 0

    position: Qt.vector3d(0, 0, 0)
    rotation: Qt.quaternion(1, 0, 0, 0)

    PerspectiveCamera {
        id: camera

        position: Qt.vector3d(
            0,
            400,
            1600
        )

        clipNear: 0.05
        clipFar: 2.0e4
    }

    function follow(i) {
        if (i < 0 || i >= frontManager.heavyCount)
            return

        var bodyPosition = frontManager.heavyPosition(i)

        root.position = Qt.vector3d(0, 0, 0)
        root.rotation = Qt.quaternion(1, 0, 0, 0)

        followOffset = Qt.vector3d(0, 0, followDistance)
        camera.position = bodyPosition.plus(followOffset)
        camera.lookAt(bodyPosition)

        followedIndex = i

        frameTick++
    }

    function release() {
        if (followedIndex < 0)
            return

        // On récupère d'abord la transformation MONDE de la caméra.
        var worldPosition = camera.scenePosition
        var worldRotation = camera.sceneRotation

        // Suppression du pivot.
        root.position = Qt.vector3d(0, 0, 0)
        root.rotation = Qt.quaternion(1, 0, 0, 0)

        // La caméra est maintenant directement dans le monde.
        camera.position = worldPosition
        camera.rotation = worldRotation

        followedIndex = -1

        frameTick++
    }

    FrameAnimation {
        running: true

        onTriggered: {
            if (root.followedIndex >= 0) {
                camera.position = frontManager.heavyPosition(
                    root.followedIndex
                ).plus(root.followOffset)
            }

            root.frameTick++
        }
    }
}
