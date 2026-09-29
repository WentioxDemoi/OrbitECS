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

    property bool menuOpen: false
    property real menuProgress: menuOpen ? 1.0 : 0.0

    Behavior on menuProgress {
        NumberAnimation {
            duration: 260
            easing.type: Easing.OutCubic
        }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: rig.release()
    }

    // Pivot caméra + logique de suivi d'astre.
    // Sibling du View3D : ce n'est pas un nœud visible, seul son
    // impact sur la transformation de "camera" compte.
    CameraRig {
        id: rig
    }

    View3D {
        id: sceneView

        anchors.fill: parent
        camera: rig.camera

        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#020308"

            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(
                -35,
                -45,
                0
            )

            brightness: 1.0
        }

        // Corps massifs : Soleil, planètes, lunes...
        Model {
            source: "#Sphere"
            instancing: frontManager.heavyInstancing

            scale: Qt.vector3d(
                0.03,
                0.03,
                0.03
            )

            materials: PrincipledMaterial {
                baseColor: "white"
                lighting: PrincipledMaterial.NoLighting

                emissiveFactor: Qt.vector3d(
                    0.5,
                    0.5,
                    0.5
                )
            }
        }

        // Corps légers : astéroïdes
        Model {
            source: "#Sphere"
            instancing: frontManager.lightInstancing

            scale: Qt.vector3d(
                0.03,
                0.03,
                0.03
            )

            materials: PrincipledMaterial {
                baseColor: "#b8c0d0"
                lighting: PrincipledMaterial.NoLighting
            }
        }
    }

    BodyLabels {
        camera: rig.camera
        sceneView: sceneView
        followedIndex: rig.followedIndex
        frameTick: rig.frameTick
    }

    WasdController {
        anchors.fill: parent

        controlledObject: rig.camera

        speed: 1.0
        shiftSpeed: 4.0

        forwardSpeed: 1.0
        backSpeed: 1.0

        rightSpeed: 1.0
        leftSpeed: 1.0

        upSpeed: 10.0
        downSpeed: 10.0

        xSpeed: 0.2
        ySpeed: 0.2
    }

    WheelHandler {
        orientation: Qt.Vertical
        target: null

        acceptedDevices:
            PointerDevice.Mouse |
            PointerDevice.TouchPad

        onWheel: function(event) {
            var delta = -event.angleDelta.y * 0.01

            var p = rig.camera.position

            var k = 1.0 + 0.1 * delta

            k = Math.max(k, 0.05)

            var len = p.length() * k

            if (len < 0.3 || len > 2.0e4)
                return

            if (rig.followedIndex >= 0) {
                rig.followOffset = rig.followOffset.times(k)
                rig.followDistance = rig.followOffset.length()
                rig.camera.position = frontManager.heavyPosition(
                    rig.followedIndex
                ).plus(rig.followOffset)
            } else {
                rig.camera.position = p.times(k)
            }
        }
    }

    SidePanel {
        id: menuPanel

        backdrop: sceneView

        width: 210

        height:
            Math.min(
                contentHeight + 16,
                window.height - 100
            ) * window.menuProgress

        x: window.width - width - 16
        y: 16 + 52 + 10

        visible: window.menuProgress > 0.01
        opacity: window.menuProgress

        followedIndex: rig.followedIndex

        onBodySelected: (index) => rig.follow(index)
        onFreeViewRequested: rig.release()
    }

    ToggleButton {
        backdrop: sceneView

        x: window.width - width - 16
        y: 16

        menuOpen: window.menuOpen

        onToggled: window.menuOpen = !window.menuOpen
    }

    Column {
        spacing: 12

        width: Math.min(window.width - 64, 640)

        x: (window.width - width) / 2
        y: window.height - height - 24

        SpeedControl {
            id: speedControl
            width: parent.width
        }

        NumericSlider {
            width: parent.width

            label: "dt"
            suffix: " s"

            from: 1
            to: 3600
            stepSize: 1
            value: frontManager.dt

            onValueChanged: {
                speedControl.ensureAtLeast(value)
            }
        }

        NumericSlider {
            width: parent.width

            label: "Astéroïdes"

            from: 0
            to: 100000
            stepSize: 100
            value: 1000

            onValueChanged: frontManager.lightInstancing.instanceCountOverride = value
        }
    }
}
