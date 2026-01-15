import QtQuick
import ".."

Item {
    id: root

    Repeater {
        model: 80

        Rectangle {
            property real starX: Math.random()
            property real starY: Math.random()
            property real starSize: Math.random() * 2 + 1
            property real baseOpacity: Math.random() * 0.4 + 0.2

            x: starX * root.width
            y: starY * root.height
            width: starSize
            height: starSize
            radius: starSize / 2
            color: "white"
            opacity: baseOpacity

            SequentialAnimation on opacity {
                running: true
                loops: Animation.Infinite

                NumberAnimation {
                    to: baseOpacity * 0.3
                    duration: Math.random() * 2000 + 1500
                    easing.type: Easing.InOutSine
                }
                NumberAnimation {
                    to: baseOpacity
                    duration: Math.random() * 2000 + 1500
                    easing.type: Easing.InOutSine
                }
            }
        }
    }

    // Shooting star occasionally
    Rectangle {
        id: shootingStar
        width: 3
        height: 3
        radius: 1.5
        color: Theme.portalGreen
        opacity: 0
        x: -50
        y: -50

        Rectangle {
            width: 30
            height: 2
            anchors.right: parent.left
            anchors.verticalCenter: parent.verticalCenter
            radius: 1

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 1.0; color: Theme.portalGreen }
            }
        }

        SequentialAnimation {
            running: true
            loops: Animation.Infinite

            PauseAnimation { duration: Math.random() * 8000 + 5000 }

            ParallelAnimation {
                NumberAnimation {
                    target: shootingStar
                    property: "opacity"
                    from: 0; to: 1
                    duration: 100
                }
                NumberAnimation {
                    target: shootingStar
                    property: "x"
                    from: root.width * 0.8
                    to: root.width * 0.2
                    duration: 800
                    easing.type: Easing.Linear
                }
                NumberAnimation {
                    target: shootingStar
                    property: "y"
                    from: root.height * 0.1
                    to: root.height * 0.4
                    duration: 800
                    easing.type: Easing.Linear
                }
            }

            NumberAnimation {
                target: shootingStar
                property: "opacity"
                to: 0
                duration: 100
            }
        }
    }
}
