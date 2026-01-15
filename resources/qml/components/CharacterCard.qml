import QtQuick
import QtQuick.Layouts
import ".."

Item {
    id: root
    width: 260
    height: 340

    // Required properties from model - names must match role names exactly
    required property int index
    required property int id
    required property string name
    required property string status
    required property string species
    required property string type
    required property string gender
    required property string originName
    required property int originId
    required property string locationName
    required property int locationId
    required property string imageUrl
    required property int episodeCount
    required property string url
    required property string created

    property bool revealed: false

    // Background card
    Rectangle {
        id: card
        anchors.fill: parent
        radius: Theme.radiusLarge
        color: Theme.spaceDark
        border.width: 2
        border.color: mouseArea.containsMouse
            ? Theme.portalGreen
            : Qt.rgba(Theme.portalGreen.r, Theme.portalGreen.g, Theme.portalGreen.b, 0.3)

        Behavior on border.color { ColorAnimation { duration: 200 } }

        // Hover glow
        Rectangle {
            anchors.fill: parent
            anchors.margins: -4
            radius: parent.radius + 4
            color: "transparent"
            border.width: 8
            border.color: Theme.portalGreen
            opacity: mouseArea.containsMouse ? 0.2 : 0
            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacingMedium
            spacing: Theme.spacingSmall

            // Character image with portal frame
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.cardImageSize + 20
                Layout.alignment: Qt.AlignHCenter

                // Portal frame
                Rectangle {
                    id: portalFrame
                    anchors.centerIn: parent
                    width: Theme.cardImageSize
                    height: Theme.cardImageSize
                    radius: width / 2
                    color: "transparent"
                    border.width: Theme.isCompact ? 2 : 3
                    border.color: Theme.portalGreen

                    // Animated border glow
                    SequentialAnimation on border.color {
                        running: root.revealed
                        loops: Animation.Infinite
                        ColorAnimation { to: Theme.portalGreenGlow; duration: 1500; easing.type: Easing.InOutSine }
                        ColorAnimation { to: Theme.portalGreenDark; duration: 1500; easing.type: Easing.InOutSine }
                    }

                    // Character image (hidden, source for shader)
                    Image {
                        id: characterImage
                        anchors.centerIn: parent
                        width: parent.width - 12
                        height: parent.height - 12
                        source: root.imageUrl
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        visible: false
                    }

                    // Circular masked image using compiled shader
                    ShaderEffect {
                        id: circularImage
                        anchors.centerIn: parent
                        width: characterImage.width
                        height: characterImage.height
                        visible: characterImage.status === Image.Ready

                        property variant source: characterImage

                        fragmentShader: "qrc:/shaders/circlemask.frag.qsb"
                    }

                    // Placeholder while loading
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width - 12
                        height: parent.height - 12
                        radius: width / 2
                        color: Theme.spacePurple
                        visible: characterImage.status !== Image.Ready

                        Text {
                            anchors.centerIn: parent
                            text: root.name.charAt(0).toUpperCase()
                            color: Theme.portalGreen
                            font.family: Theme.fontTitle
                            font.pixelSize: Theme.fontSizeTitle
                        }
                    }

                    // Green border highlight
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width - 12
                        height: parent.height - 12
                        radius: width / 2
                        color: "transparent"
                        border.width: 2
                        border.color: Theme.portalGreen
                    }
                }

                // Status indicator
                Rectangle {
                    anchors.right: portalFrame.right
                    anchors.bottom: portalFrame.bottom
                    anchors.margins: 8
                    width: 24
                    height: 24
                    radius: 12
                    color: root.status === "Alive" ? Theme.statusAlive :
                           root.status === "Dead" ? Theme.statusDead :
                           Theme.statusUnknown
                    border.width: 2
                    border.color: Theme.spaceDark

                    // Pulse animation for "Alive"
                    SequentialAnimation on scale {
                        running: root.status === "Alive" && root.revealed
                        loops: Animation.Infinite
                        NumberAnimation { to: 1.2; duration: 800; easing.type: Easing.OutQuad }
                        NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InQuad }
                    }
                }
            }

            // Character name
            Text {
                Layout.fillWidth: true
                text: root.name
                color: Theme.textPrimary
                font.family: Theme.fontBody
                font.pixelSize: Theme.fontSizeTitle
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                maximumLineCount: 2
                wrapMode: Text.WordWrap
            }

            // Species, gender and status
            Text {
                Layout.fillWidth: true
                text: root.status + " • " + root.species + (root.type !== "" ? " (" + root.type + ")" : "")
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeMedium
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }

            // Gender
            Text {
                Layout.fillWidth: true
                text: root.gender
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSmall
                horizontalAlignment: Text.AlignHCenter
                visible: root.gender !== "unknown"
            }

            // Location info
            Text {
                Layout.fillWidth: true
                text: "📍 " + root.locationName
                color: Theme.textMuted
                font.pixelSize: Theme.fontSizeSmall
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                visible: root.locationName !== "" && root.locationName !== "unknown"
            }

            Item { Layout.fillHeight: true }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: root.clicked(root.id, root.name, root.status, root.species, root.type, root.gender, root.originName, root.locationName, root.imageUrl, root.episodeCount, root.created)
        }
    }

    // Portal reveal animation overlay
    Rectangle {
        id: revealOverlay
        anchors.centerIn: parent
        width: revealed ? Math.max(root.width, root.height) * 2 : 0
        height: width
        radius: width / 2
        color: Theme.portalGreen
        opacity: revealed ? 0 : 0.9
        visible: opacity > 0

        Behavior on width {
            NumberAnimation {
                duration: 400
                easing.type: Easing.OutCubic
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.OutQuad
            }
        }
    }

    // Scale animation on appear
    scale: revealed ? 1 : 0.8
    opacity: revealed ? 1 : 0

    Behavior on scale {
        NumberAnimation {
            duration: 350
            easing.type: Easing.OutBack
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 300
        }
    }

    // Trigger reveal on component completion
    Component.onCompleted: {
        revealTimer.start()
    }

    Timer {
        id: revealTimer
        interval: 50 + Math.random() * 150
        onTriggered: root.revealed = true
    }

    signal clicked(int charId, string charName, string charStatus, string charSpecies, string charType, string charGender, string charOrigin, string charLocation, string charImageUrl, int charEpisodeCount, string charCreated)
}
