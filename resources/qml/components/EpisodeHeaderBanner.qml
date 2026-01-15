import QtQuick
import QtQuick.Layouts
import ".."

Rectangle {
    id: root
    width: parent.width
    height: Theme.bannerHeight
    radius: Theme.radiusMedium
    color: Qt.rgba(Theme.spaceDark.r, Theme.spaceDark.g, Theme.spaceDark.b, 0.9)
    border.width: 2
    border.color: Theme.portalGreen
    clip: true

    property string episodeCode: ""
    property string episodeName: ""
    property bool isVisible: episodeCode !== ""

    visible: isVisible
    opacity: isVisible ? 1 : 0

    Behavior on opacity {
        NumberAnimation { duration: 300 }
    }

    // Animated portal/flame background
    Item {
        id: portalBackground
        anchors.fill: parent
        opacity: 0.3

        // Base gradient
        Rectangle {
            anchors.fill: parent
            radius: root.radius
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 0.5; color: Qt.rgba(Theme.portalGreen.r, Theme.portalGreen.g, Theme.portalGreen.b, 0.15) }
                GradientStop { position: 1.0; color: "transparent" }
            }
        }

        // Rotating outer ring
        Rectangle {
            id: outerRing
            anchors.left: parent.left
            anchors.leftMargin: -width * 0.3
            anchors.verticalCenter: parent.verticalCenter
            width: parent.height * 2
            height: width
            radius: width / 2
            color: "transparent"
            border.width: 3
            border.color: Theme.portalGreen
            opacity: 0.4

            RotationAnimation on rotation {
                running: root.isVisible
                from: 0
                to: 360
                duration: 4000
                loops: Animation.Infinite
            }

            // Arc accent
            Rectangle {
                width: parent.width * 0.3
                height: parent.border.width + 2
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                radius: height / 2
                color: Theme.portalGreenGlow
            }
        }

        // Counter-rotating middle ring
        Rectangle {
            anchors.left: parent.left
            anchors.leftMargin: -width * 0.2
            anchors.verticalCenter: parent.verticalCenter
            width: parent.height * 1.5
            height: width
            radius: width / 2
            color: "transparent"
            border.width: 2
            border.color: Theme.portalGreenDark
            opacity: 0.3

            RotationAnimation on rotation {
                running: root.isVisible
                from: 360
                to: 0
                duration: 3000
                loops: Animation.Infinite
            }
        }

        // Flame/particle effects rising upward
        Repeater {
            model: 12

            Rectangle {
                id: flameParticle
                property real startX: Math.random() * portalBackground.width
                property real particleDelay: Math.random() * 2000

                x: startX
                y: portalBackground.height
                width: 4 + Math.random() * 4
                height: width
                radius: width / 2
                color: Theme.portalGreen
                opacity: 0

                SequentialAnimation {
                    running: root.isVisible
                    loops: Animation.Infinite

                    PauseAnimation { duration: flameParticle.particleDelay }

                    ParallelAnimation {
                        NumberAnimation {
                            target: flameParticle
                            property: "y"
                            from: root.height + 10
                            to: -10
                            duration: 1500 + Math.random() * 1000
                            easing.type: Easing.OutQuad
                        }
                        NumberAnimation {
                            target: flameParticle
                            property: "x"
                            from: flameParticle.startX
                            to: flameParticle.startX + (Math.random() - 0.5) * 40
                            duration: 1500 + Math.random() * 1000
                        }
                        SequentialAnimation {
                            NumberAnimation {
                                target: flameParticle
                                property: "opacity"
                                from: 0
                                to: 0.8
                                duration: 300
                            }
                            NumberAnimation {
                                target: flameParticle
                                property: "opacity"
                                to: 0
                                duration: 1200 + Math.random() * 1000
                                easing.type: Easing.InQuad
                            }
                        }
                    }
                }
            }
        }

        // Color cycling glow overlay
        Rectangle {
            anchors.fill: parent
            radius: root.radius
            opacity: 0.15

            SequentialAnimation on color {
                running: root.isVisible
                loops: Animation.Infinite
                ColorAnimation { to: Theme.portalGreen; duration: 1000 }
                ColorAnimation { to: Theme.portalGreenGlow; duration: 1000 }
                ColorAnimation { to: Theme.portalGreenDark; duration: 1000 }
            }
        }
    }

    // Content
    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingMedium

        // Episode code badge
        Rectangle {
            Layout.preferredWidth: Theme.isCompact ? 70 : 90
            Layout.preferredHeight: Theme.isCompact ? 36 : 46
            radius: Theme.radiusSmall
            color: Theme.spacePurple
            border.width: 2
            border.color: Theme.dimensionPurple

            Text {
                anchors.centerIn: parent
                text: root.episodeCode
                color: Theme.portalGreen
                font.family: Theme.fontDisplay
                font.pixelSize: Theme.isCompact ? 18 : 24
            }
        }

        // Episode title
        Text {
            Layout.fillWidth: true
            text: root.episodeName
            color: Theme.textPrimary
            font.family: Theme.fontTitle
            font.pixelSize: Theme.isCompact ? Theme.bannerTitleSize + 4 : Theme.bannerTitleSize + 6
            elide: Text.ElideRight
        }
    }

    // Bottom accent line
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 2

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 0.3; color: Theme.portalGreen }
            GradientStop { position: 0.7; color: Theme.portalGreen }
            GradientStop { position: 1.0; color: "transparent" }
        }

        SequentialAnimation on opacity {
            running: root.isVisible
            loops: Animation.Infinite
            NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutSine }
            NumberAnimation { to: 0.5; duration: 1000; easing.type: Easing.InOutSine }
        }
    }
}
