import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import QtQuick.Effects

Window {
    id: window

    width: 1280
    height: 800
    visible: true
    title: "OrbitECS"
    color: "black"

    // --- Suivi d'un astre -----------------------------------------------

    property int frameTick: 0
    property int followedIndex: -1

    // Distance caméra-astre en mode suivi
    property real followDistance: 0.5
    property vector3d followOffset: Qt.vector3d(0, 0, 0.5)

    property bool menuOpen: false
    property real menuProgress: menuOpen ? 1.0 : 0.0

    Behavior on menuProgress {
        NumberAnimation {
            duration: 260
            easing.type: Easing.OutCubic
        }
    }

    // ---------------------------------------------------------------------
    // Suivi
    //
    // originNode est uniquement le pivot de la caméra.
    // Les Model/instances restent dans la scène et ne sont jamais déplacés.
    // ---------------------------------------------------------------------

    function follow(i) {
        if (i < 0 || i >= frontManager.heavyCount)
            return

        var bodyPosition = frontManager.heavyPosition(i)

        originNode.position = Qt.vector3d(0, 0, 0)
        originNode.rotation = Qt.quaternion(1, 0, 0, 0)

        followOffset = Qt.vector3d(0, 0, followDistance)
        camera.position = bodyPosition.plus(followOffset)
        camera.lookAt(bodyPosition)

        followedIndex = i

        frameTick++
    }

    // ---------------------------------------------------------------------
    // Quitter le suivi
    // ---------------------------------------------------------------------

    function release() {
        if (followedIndex < 0)
            return

        // On récupère d'abord la transformation MONDE de la caméra.
        var worldPosition = camera.scenePosition
        var worldRotation = camera.sceneRotation

        // Suppression du pivot.
        originNode.position = Qt.vector3d(0, 0, 0)
        originNode.rotation = Qt.quaternion(1, 0, 0, 0)

        // La caméra est maintenant directement dans le monde.
        camera.position = worldPosition
        camera.rotation = worldRotation

        followedIndex = -1

        frameTick++
    }

    // ---------------------------------------------------------------------
    // Mise à jour du pivot lorsque l'astre se déplace
    // ---------------------------------------------------------------------

    FrameAnimation {
        running: true

        onTriggered: {
            if (window.followedIndex >= 0) {
                camera.position = frontManager.heavyPosition(
                    window.followedIndex
                ).plus(window.followOffset)
            }

            window.frameTick++
        }
    }

    Shortcut {
        sequence: "Escape"
        onActivated: window.release()
    }

    // --- Composant Liquid Glass ------------------------------------------

    component GlassBackground: Item {
        id: gb

        property Item backdrop
        property real cornerRadius: 20

        default property alias content: contentItem.data

        ShaderEffectSource {
            id: src

            anchors.fill: parent
            sourceItem: gb.backdrop

            sourceRect: Qt.rect(
                gb.x,
                gb.y,
                gb.width,
                gb.height
            )

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

    // --- Scène 3D --------------------------------------------------------

    Node {
        id: originNode

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

    // --- Labels ----------------------------------------------------------

    Repeater {
        model: frontManager.heavyCount

        delegate: Text {
            id: bodyName

            property vector3d viewportPosition: {
                window.frameTick

                return camera.mapToViewport(
                    frontManager.heavyPosition(index)
                )
            }

            x: viewportPosition.x * sceneView.width - width / 2

            y: viewportPosition.y * sceneView.height - height - 8

            text: frontManager.heavyName(index)

            color: index === window.followedIndex
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

    // --- Contrôles caméra ------------------------------------------------

    WasdController {
        id: cameraController

        anchors.fill: parent

        controlledObject: camera

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

    // --- Zoom ------------------------------------------------------------

    WheelHandler {
        id: cameraZoom

        orientation: Qt.Vertical
        target: null

        acceptedDevices:
            PointerDevice.Mouse |
            PointerDevice.TouchPad

        onWheel: function(event) {
            var delta = -event.angleDelta.y * 0.01

            var p = camera.position

            var k = 1.0 + 0.1 * delta

            k = Math.max(k, 0.05)

            var len = p.length() * k

            if (len < 0.3 || len > 2.0e4)
                return

            if (window.followedIndex >= 0) {
                window.followOffset = window.followOffset.times(k)
                window.followDistance = window.followOffset.length()
                camera.position = frontManager.heavyPosition(
                    window.followedIndex
                ).plus(window.followOffset)
            } else {
                camera.position = p.times(k)
            }
        }
    }

    // --- Menu Liquid Glass ----------------------------------------------

    GlassBackground {
        id: menuPanel

        backdrop: sceneView
        cornerRadius: 24

        width: 210

        height:
            Math.min(
                menuList.contentHeight + 16,
                window.height - 100
            ) * window.menuProgress

        x: window.width - width - 16
        y: 16 + 52 + 10

        visible: window.menuProgress > 0.01
        opacity: window.menuProgress

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

                readonly property bool isFree:
                    window.followedIndex < 0

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

                        cursorShape:
                            Qt.PointingHandCursor

                        onClicked: window.release()
                    }
                }
            }

            delegate: Item {
                id: entry

                width: menuList.width
                height: 40

                readonly property bool isFollowed:
                    index === window.followedIndex

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

                        text:
                            frontManager.heavyName(index)

                        color: "white"

                        font.pixelSize: 14
                        font.bold: entry.isFollowed
                    }

                    MouseArea {
                        id: area

                        anchors.fill: parent

                        hoverEnabled: true

                        cursorShape:
                            Qt.PointingHandCursor

                        onClicked:
                            window.follow(index)
                    }
                }
            }
        }
    }

    // --- Bouton rond -----------------------------------------------------

    GlassBackground {
        id: toggleButton

        backdrop: sceneView

        width: 52
        height: 52

        cornerRadius: 26

        x: window.width - width - 16
        y: 16

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
                window.menuOpen
                ? "✕"
                : "☰"

            color: "white"

            font.pixelSize: 22
        }

        MouseArea {
            id: toggleArea

            anchors.fill: parent

            cursorShape:
                Qt.PointingHandCursor

            onClicked:
                window.menuOpen =
                    !window.menuOpen
        }
    }
}