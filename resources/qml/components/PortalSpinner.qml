import QtQuick
import ".."

Item {
    id: root
    width: 80
    height: 80
    property bool running: true

    // Outer ring
    Rectangle {
        id: outerRing
        anchors.fill: parent
        radius: width / 2
        color: "transparent"
        border.width: 3
        border.color: Theme.portalGreen

        RotationAnimation on rotation {
            running: root.running
            from: 0
            to: 360
            duration: 2000
            loops: Animation.Infinite
        }

        // Gradient arc effect
        Rectangle {
            width: parent.width * 0.3
            height: parent.border.width + 2
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            radius: height / 2
            color: Theme.portalGreenGlow
        }
    }

    // Middle ring
    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 0.7
        height: width
        radius: width / 2
        color: "transparent"
        border.width: 2
        border.color: Theme.portalGreenDark
        opacity: 0.6

        RotationAnimation on rotation {
            running: root.running
            from: 360
            to: 0
            duration: 1500
            loops: Animation.Infinite
        }
    }

    // Inner pulsing core
    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 0.35
        height: width
        radius: width / 2
        color: Theme.portalGreen
        opacity: 0.6

        SequentialAnimation on scale {
            running: root.running
            loops: Animation.Infinite
            NumberAnimation { to: 1.3; duration: 600; easing.type: Easing.OutQuad }
            NumberAnimation { to: 0.8; duration: 600; easing.type: Easing.InQuad }
        }

        SequentialAnimation on opacity {
            running: root.running
            loops: Animation.Infinite
            NumberAnimation { to: 0.9; duration: 600 }
            NumberAnimation { to: 0.3; duration: 600 }
        }
    }

    // Glow particles
    Repeater {
        model: 8

        Rectangle {
            property real angle: index * 45
            property real distance: root.width * 0.45

            x: root.width / 2 + Math.cos(angle * Math.PI / 180) * distance - width / 2
            y: root.height / 2 + Math.sin(angle * Math.PI / 180) * distance - height / 2
            width: 6
            height: 6
            radius: 3
            color: Theme.portalGreen
            opacity: 0.7

            SequentialAnimation on opacity {
                running: root.running
                loops: Animation.Infinite
                NumberAnimation { to: 1.0; duration: 300 + index * 100 }
                NumberAnimation { to: 0.2; duration: 300 + index * 100 }
            }
        }
    }
}
