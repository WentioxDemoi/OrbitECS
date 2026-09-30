import QtQuick
import QtQuick.Controls

Column {
    id: speedControl

    spacing: 4

    readonly property var sliderValues: [
        {
            label: "-3j/s",
            seconds: -3 * 24 * 3600
        },
        {
            label: "-2j/s",
            seconds: -2 * 24 * 3600
        },
        {
            label: "-1j/s",
            seconds: -24 * 3600
        },
        {
            label: "-12h/s",
            seconds: -12 * 3600
        },
        {
            label: "-6h/s",
            seconds: -6 * 3600
        },
        {
            label: "-3h/s",
            seconds: -3 * 3600
        },
        {
            label: "-2h/s",
            seconds: -2 * 3600
        },
        {
            label: "-1h/s",
            seconds: -3600
        },
        {
            label: "-30m/s",
            seconds: -30 * 60
        },
        {
            label: "-15m/s",
            seconds: -15 * 60
        },
        {
            label: "Temps réel",
            seconds: 1
        },
        {
            label: "+15m/s",
            seconds: 15 * 60
        },
        {
            label: "+30m/s",
            seconds: 30 * 60
        },
        {
            label: "+1h/s",
            seconds: 3600
        },
        {
            label: "+2h/s",
            seconds: 2 * 3600
        },
        {
            label: "+3h/s",
            seconds: 3 * 3600
        },
        {
            label: "+6h/s",
            seconds: 6 * 3600
        },
        {
            label: "+12h/s",
            seconds: 12 * 3600
        },
        {
            label: "+1j/s",
            seconds: 24 * 3600
        },
        {
            label: "+2j/s",
            seconds: 2 * 24 * 3600
        },
        {
            label: "+3j/s",
            seconds: 3 * 24 * 3600
        }
    ]

    Label {
        anchors.horizontalCenter: parent.horizontalCenter

        text: speedControl.sliderValues[Math.round(simSpeedSlider.visualPosition * (speedControl.sliderValues.length - 1))].label
    }

    // Vitesse de simulation signée.
    readonly property int currentSeconds: sliderValues[Math.round(simSpeedSlider.value)].seconds

    // Vitesse absolue utilisée pour déterminer le DT.
    readonly property int currentSpeedMagnitude: Math.abs(currentSeconds)

    onCurrentSpeedMagnitudeChanged: {
        // DT est toujours positif.
        // Le signe de simSpeedFactor sert uniquement au sens
        // d'évolution de simTime.
        if (currentSpeedMagnitude < Math.abs(frontManager.dt)) {
            frontManager.userSetDt(currentSpeedMagnitude);
        }
    }

    Slider {
        id: simSpeedSlider

        width: speedControl.width

        live: false

        from: 0
        to: speedControl.sliderValues.length - 1
        stepSize: 1
        value: 10

        onValueChanged: {
            var index = Math.round(value);
            frontManager.simSpeedFactor = speedControl.sliderValues[index].seconds;
        }
    }

    function ensureAtLeast(minSeconds) {
        minSeconds = Math.abs(minSeconds);

        var current = currentSeconds;
        var direction = current < 0 ? -1 : 1;

        var bestIndex = -1;
        var bestMagnitude = Number.MAX_VALUE;

        for (var i = 0; i < sliderValues.length; i++) {
            var candidate = sliderValues[i].seconds;

            // On conserve le sens actuel du temps.
            if ((candidate < 0 ? -1 : 1) !== direction)
                continue;
            var magnitude = Math.abs(candidate);

            if (magnitude >= minSeconds && magnitude < bestMagnitude) {
                bestMagnitude = magnitude;
                bestIndex = i;
            }
        }

        if (bestIndex >= 0) {
            simSpeedSlider.value = bestIndex;
        }
    }
}
