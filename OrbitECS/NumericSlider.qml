import QtQuick
import QtQuick.Controls

Column {
    id: control

    property alias from: slider.from
    property alias to: slider.to
    property alias stepSize: slider.stepSize
    property alias value: slider.value

    property string label: ""
    property string suffix: ""

    spacing: 4

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        text: control.label + " : " + Math.round(
            control.from + slider.visualPosition * (control.to - control.from)
        ) + control.suffix
    }

    Slider {
        id: slider
        width: control.width
        live: false // update uniquement au relâchement, comme SpeedControl
    }
}
