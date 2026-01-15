import QtQuick
import QtQuick.Layouts
import ".."

Item {
    id: root
    anchors.fill: parent
    visible: false
    z: 500

    // Character data
    property int characterId: -1
    property string characterName: ""
    property string characterStatus: ""
    property string characterSpecies: ""
    property string characterType: ""
    property string characterGender: ""
    property string characterOrigin: ""
    property string characterLocation: ""
    property string characterImage: ""
    property int characterEpisodeCount: 0
    property string characterCreated: ""

    function show(charId, charName, charStatus, charSpecies, charType, charGender, charOrigin, charLocation, charImageUrl, charEpisodeCount, charCreated) {
        root.characterId = charId
        root.characterName = charName
        root.characterStatus = charStatus
        root.characterSpecies = charSpecies
        root.characterType = charType
        root.characterGender = charGender
        root.characterOrigin = charOrigin
        root.characterLocation = charLocation
        root.characterImage = charImageUrl
        root.characterEpisodeCount = charEpisodeCount
        root.characterCreated = charCreated
        root.visible = true
        showAnimation.start()
    }

    function hide() {
        hideAnimation.start()
    }

    // Backdrop
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: root.visible ? 0.7 : 0

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.hide()
        }
    }

    // Modal container
    Rectangle {
        id: modal
        anchors.centerIn: parent
        width: Theme.isCompact ? parent.width * 0.9 : 400
        height: contentColumn.implicitHeight + Theme.spacingXLarge * 2
        radius: 20
        color: Theme.spaceDark
        border.width: 2
        border.color: Theme.portalGreen

        scale: 0.9
        opacity: 0

        // Close button
        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: Theme.spacingMedium
            width: 36
            height: 36
            radius: 18
            color: closeMouseArea.containsMouse ? Qt.rgba(Theme.statusDead.r, Theme.statusDead.g, Theme.statusDead.b, 0.3) : "transparent"
            z: 10

            Text {
                anchors.centerIn: parent
                text: "\u2715"
                color: Theme.textSecondary
                font.pixelSize: 18
                font.bold: true
            }

            MouseArea {
                id: closeMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.hide()
            }
        }

        ColumnLayout {
            id: contentColumn
            anchors.fill: parent
            anchors.margins: Theme.spacingXLarge
            spacing: Theme.spacingLarge

            // Character image
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: Theme.isCompact ? 150 : 200

                Rectangle {
                    id: imageFrame
                    anchors.centerIn: parent
                    width: Theme.isCompact ? 150 : 200
                    height: width
                    radius: width / 2
                    color: "transparent"
                    border.width: 3
                    border.color: Theme.portalGreen

                    Image {
                        id: characterImage
                        anchors.centerIn: parent
                        width: parent.width - 12
                        height: parent.height - 12
                        source: root.characterImage
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        visible: false
                    }

                    ShaderEffect {
                        anchors.centerIn: parent
                        width: characterImage.width
                        height: characterImage.height
                        visible: characterImage.status === Image.Ready
                        property variant source: characterImage
                        fragmentShader: "qrc:/shaders/circlemask.frag.qsb"
                    }

                    // Placeholder
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width - 12
                        height: parent.height - 12
                        radius: width / 2
                        color: Theme.spacePurple
                        visible: characterImage.status !== Image.Ready

                        Text {
                            anchors.centerIn: parent
                            text: root.characterName.charAt(0).toUpperCase()
                            color: Theme.portalGreen
                            font.pixelSize: Theme.fontSizeHeader
                            font.bold: true
                        }
                    }

                    // Status indicator
                    Rectangle {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 8
                        width: 28
                        height: 28
                        radius: 14
                        color: root.characterStatus === "Alive" ? Theme.statusAlive :
                               root.characterStatus === "Dead" ? Theme.statusDead :
                               Theme.statusUnknown
                        border.width: 3
                        border.color: Theme.spaceDark
                    }
                }
            }

            // Character name
            Text {
                Layout.fillWidth: true
                text: root.characterName
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeTitle
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            // Divider
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.portalGreen
                opacity: 0.2
            }

            // Info grid
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: Theme.spacingLarge
                rowSpacing: Theme.spacingMedium

                // Status
                Text {
                    text: "Status"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeMedium
                }
                RowLayout {
                    spacing: Theme.spacingSmall
                    Rectangle {
                        width: 10
                        height: 10
                        radius: 5
                        color: root.characterStatus === "Alive" ? Theme.statusAlive :
                               root.characterStatus === "Dead" ? Theme.statusDead :
                               Theme.statusUnknown
                    }
                    Text {
                        text: root.characterStatus
                        color: Theme.textPrimary
                        font.pixelSize: Theme.fontSizeMedium
                    }
                }

                // Species
                Text {
                    text: "Species"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeMedium
                }
                Text {
                    Layout.fillWidth: true
                    text: root.characterSpecies + (root.characterType !== "" ? " (" + root.characterType + ")" : "")
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeMedium
                    elide: Text.ElideRight
                }

                // Gender
                Text {
                    text: "Gender"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeMedium
                }
                Text {
                    text: root.characterGender
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeMedium
                }

                // Origin
                Text {
                    text: "Origin"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeMedium
                }
                Text {
                    Layout.fillWidth: true
                    text: root.characterOrigin !== "" ? root.characterOrigin : "Unknown"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeMedium
                    elide: Text.ElideRight
                }

                // Location
                Text {
                    text: "Location"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeMedium
                }
                Text {
                    Layout.fillWidth: true
                    text: root.characterLocation !== "" ? root.characterLocation : "Unknown"
                    color: Theme.textPrimary
                    font.pixelSize: Theme.fontSizeMedium
                    elide: Text.ElideRight
                }

                // Episode count
                Text {
                    text: "Episodes"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeMedium
                }
                Text {
                    text: root.characterEpisodeCount.toString()
                    color: Theme.meeseeksBlue
                    font.pixelSize: Theme.fontSizeMedium
                    font.bold: true
                }
            }
        }
    }

    // Show animation
    ParallelAnimation {
        id: showAnimation
        NumberAnimation {
            target: modal
            property: "scale"
            from: 0.9
            to: 1.0
            duration: 200
            easing.type: Easing.OutQuad
        }
        NumberAnimation {
            target: modal
            property: "opacity"
            from: 0
            to: 1
            duration: 200
        }
    }

    // Hide animation
    SequentialAnimation {
        id: hideAnimation
        ParallelAnimation {
            NumberAnimation {
                target: modal
                property: "scale"
                to: 0.9
                duration: 150
                easing.type: Easing.InQuad
            }
            NumberAnimation {
                target: modal
                property: "opacity"
                to: 0
                duration: 150
            }
        }
        ScriptAction {
            script: root.visible = false
        }
    }
}
