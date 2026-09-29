import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Node {
    id: root

    property alias camera: camera

    property int followedIndex: -1

    property vector3d followOffset: Qt.vector3d(0, 400, 1600)

    property real followDistance: 1.0

    property int frameTick: 0

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

        followedIndex = i

        // On conserve la direction actuelle, mais avec une distance fixe.
        var direction = camera.scenePosition.minus(bodyPosition)
        var distance = direction.length()

        if (distance < 0.01)
            direction = Qt.vector3d(0, 0, 1)
        else
            direction = direction.times(1.0 / distance)

        followOffset = direction.times(followDistance)

        camera.position = bodyPosition.plus(followOffset)

        // Première orientation immédiate.
        camera.lookAt(bodyPosition)

        frameTick++
    }

    function adjustFollowDistance(factor) {
        if (followedIndex < 0)
            return

        followDistance = Math.max(
            0.1,
            Math.min(followDistance * factor, 10000)
        )

        var offsetLength = followOffset.length()
        if (offsetLength > 0.001)
            followOffset = followOffset.times(followDistance / offsetLength)
    }

    function release() {
        if (followedIndex < 0)
            return

        followedIndex = -1

        frameTick++
    }

    FrameAnimation {
        running: true

        onTriggered: {
            if (root.followedIndex >= 0) {
                var bodyPosition =
                    frontManager.heavyPosition(root.followedIndex)

                // Le corps bouge.
                // La caméra conserve son offset par rapport à lui.
                camera.position =
                    bodyPosition.plus(root.followOffset)

                // Et surtout : on recalcule l'orientation
                // vers le corps à chaque frame.
                camera.lookAt(bodyPosition)
            }

            root.frameTick++
        }
    }
}