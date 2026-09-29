import QtQuick
import QtQuick.Controls

Column {
    id: speedControl

    spacing: 4

    readonly property var sliderValues: [
        { label: "-3j/s",   seconds: -3 * 24 * 3600 },
        { label: "-2j/s",   seconds: -2 * 24 * 3600 },
        { label: "-1j/s",   seconds: -24 * 3600 },
        { label: "-12h/s",  seconds: -12 * 3600 },
        { label: "-6h/s",   seconds: -6 * 3600 },
        { label: "-3h/s",   seconds: -3 * 3600 },
        { label: "-2h/s",   seconds: -2 * 3600 },
        { label: "-1h/s",   seconds: -3600 },
        { label: "-30m/s",  seconds: -30 * 60 },
        { label: "-15m/s",  seconds: -15 * 60 },

        { label: "Temps réel", seconds: 1 },

        { label: "+15m/s",  seconds: 15 * 60 },
        { label: "+30m/s",  seconds: 30 * 60 },
        { label: "+1h/s",   seconds: 3600 },
        { label: "+2h/s",   seconds: 2 * 3600 },
        { label: "+3h/s",   seconds: 3 * 3600 },
        { label: "+6h/s",   seconds: 6 * 3600 },
        { label: "+12h/s",  seconds: 12 * 3600 },
        { label: "+1j/s",   seconds: 24 * 3600 },
        { label: "+2j/s",   seconds: 2 * 24 * 3600 },
        { label: "+3j/s",   seconds: 3 * 24 * 3600 }
    ]

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        text: speedControl.sliderValues[
            Math.round(simSpeedSlider.visualPosition * (speedControl.sliderValues.length - 1))
        ].label
    }

    // Valeur (en secondes) actuellement retenue par le curseur,
    // recalculée à chaque fois que simSpeedSlider.value change
    // (donc au relâchement, comme frontManager.simSpeedFactor).
    readonly property int currentSeconds: sliderValues[Math.round(simSpeedSlider.value)].seconds

    onCurrentSecondsChanged: {
        // Symétrique de ensureAtLeast() côté dt : si la vitesse choisie
        // descend sous dt, on baisse dt plutôt que de laisser
        // simSpeedFactor / dt tomber à 0 par division entière.
        if (currentSeconds < frontManager.dt) {
            frontManager.userSetDt(currentSeconds)
        }
    }

    Slider {
        id: simSpeedSlider

        width: speedControl.width

        live: false // Pour faire en sorte qu'on update que quand on relâche le curseur

        from: 0
        to: speedControl.sliderValues.length - 1
        stepSize: 1
        value: 10 // "+1h/s" (3600) : aligné sur le simSpeedFactor par défaut de BackManager

        onValueChanged: {
            var index = Math.round(value)
            frontManager.simSpeedFactor = speedControl.sliderValues[index].seconds
        }
    }

    // Force simSpeedFactor à valoir au moins minSeconds, en déplaçant
    // le curseur sur le premier palier qui l'atteint ou le dépasse.
    // Sans ça, simSpeedFactor / dt peut tomber à 0 (division entière)
    // si dt dépasse la vitesse de simulation choisie.
    function ensureAtLeast(minSeconds) {
        for (var i = 0; i < sliderValues.length; i++) {
            if (sliderValues[i].seconds >= minSeconds) {
                simSpeedSlider.value = i
                return
            }
        }
        simSpeedSlider.value = sliderValues.length - 1
    }
}
