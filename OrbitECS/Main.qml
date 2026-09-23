import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

Window {
    id: window
    width: 1280
    height: 800
    visible: true
    title: "OrbitECS"
    color: "black"

    // Node pivot utilisé comme origine par le contrôleur de caméra orbital.
    Node {
        id: originNode
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 400, 1600)
            clipNear: 1.0
            clipFar: 1.0e7
        }
    }

    View3D {
        id: sceneView
        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#020308"
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-35, -45, 0)
            brightness: 1.0
        }

        // Corps massifs (Soleil, planètes...) : peu nombreux, taille et
        // couleur dérivées de leur masse côté C++ (FrontManager).
        Model {
            source: "#Sphere"
            instancing: frontManager.heavyInstancing
            scale: Qt.vector3d(0.03, 0.03, 0.03)
            materials: PrincipledMaterial {
                baseColor: "white"
                lighting: PrincipledMaterial.NoLighting
                emissiveFactor: Qt.vector3d(0.5, 0.5, 0.5)
            }
        }

        // Corps légers (astéroïdes...) : potentiellement des milliers,
        // rendus via le même mécanisme d'instancing GPU en un seul draw call.
        Model {
            source: "#Sphere"
            instancing: frontManager.lightInstancing
            scale: Qt.vector3d(0.01, 0.01, 0.01)
            materials: PrincipledMaterial {
                baseColor: "#9098a8"
                lighting: PrincipledMaterial.NoLighting
            }
        }
    }

    Repeater {
        model: frontManager.heavyCount

        delegate: Text {
            id: bodyName

            property vector3d bodyPosition: frontManager.heavyPosition(index)
            property vector3d viewportPosition: camera.mapToViewport(bodyPosition)

            x: viewportPosition.x * sceneView.width - width / 2
            y: viewportPosition.y * sceneView.height - height - 8
            text: frontManager.heavyName(index)
            color: "white"
            font.pixelSize: 13
            font.bold: true
            style: Text.Outline
            styleColor: "black"
            visible: viewportPosition.z >= 0
                     && viewportPosition.x >= 0 && viewportPosition.x <= 1
                     && viewportPosition.y >= 0 && viewportPosition.y <= 1

            Timer {
                interval: 16
                running: window.visible
                repeat: true
                onTriggered: bodyName.bodyPosition =
                    frontManager.heavyPosition(index)
            }
        }
    }

    WasdController {
        id: cameraController
        anchors.fill: parent
        controlledObject: camera
        speed: 1.0
        shiftSpeed: 4.0
        forwardSpeed: 10.0
        backSpeed: 10.0
        rightSpeed: 10.0
        leftSpeed: 10.0
        upSpeed: 10.0
        downSpeed: 10.0
        xSpeed: 0.2
        ySpeed: 0.2
    }

    WheelHandler {
        id: cameraZoom
        orientation: Qt.Vertical
        target: null
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        onWheel: (event) => {
            const delta = -event.angleDelta.y * 0.01
            camera.z = Math.max(20, Math.min(1.0e7,
                camera.z + camera.z * 0.1 * delta))
        }
    }
}
