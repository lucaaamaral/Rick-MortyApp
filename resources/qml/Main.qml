import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 800
    minimumWidth: 320
    minimumHeight: 480
    title: "Rick and Morty Explorer"
    color: Theme.spaceBlack

    property int selectedEpisodeId: -1
    property bool drawerOpen: Theme.drawerDefaultOpen
    property var collapsedSeasons: ({})  // Track collapsed seasons: { "1": true, "3": true }

    function toggleSeason(season) {
        var newState = Object.assign({}, collapsedSeasons)
        if (newState[season]) {
            delete newState[season]
        } else {
            newState[season] = true
        }
        collapsedSeasons = newState
    }

    // Update Theme with window dimensions for responsive calculations
    onWidthChanged: {
        Theme.windowWidth = width
        // Auto-manage drawer based on breakpoints
        if (Theme.isExtraWide) {
            drawerOpen = true
        } else if (Theme.isCompact && drawerOpen) {
            // Don't auto-close on resize, user controls it
        }
    }
    onHeightChanged: Theme.windowHeight = height

    Component.onCompleted: {
        Theme.windowWidth = width
        Theme.windowHeight = height
        backend.loadEpisodes()
    }

    // Background stars
    StarField {
        anchors.fill: parent
        z: -1
    }

    // Main content - Character Grid (always visible)
    CharacterGridPanel {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        // Push content aside on medium/wide when drawer open
        anchors.leftMargin: (drawerOpen && !Theme.isCompact) ? drawer.width + Theme.spacingLarge : Theme.spacingMedium
        showHeader: true

        Behavior on anchors.leftMargin {
            NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
        }
    }

    // Dim overlay when drawer open on COMPACT only (modal style)
    Rectangle {
        id: dimOverlay
        anchors.fill: parent
        z: 50
        color: "#000000"
        opacity: (drawerOpen && Theme.isCompact) ? 0.6 : 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: drawerOpen = false
        }
    }

    // Episode Drawer (slides in/out)
    Rectangle {
        id: drawer
        width: Theme.drawerWidth
        height: parent.height
        x: drawerOpen ? 0 : -width
        z: Theme.isCompact ? 100 : 0
        color: Qt.rgba(Theme.spaceDark.r, Theme.spaceDark.g, Theme.spaceDark.b, 0.95)
        border.width: 1
        border.color: Qt.rgba(Theme.portalGreen.r, Theme.portalGreen.g, Theme.portalGreen.b, 0.3)

        Behavior on x {
            NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
        }

        // Episode list content
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacingLarge
            anchors.topMargin: Theme.headerTopMargin
            spacing: Theme.spacingMedium

            // Header
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "Episodes"
                    color: Theme.textPrimary
                    font.family: Theme.fontTitle
                    font.pixelSize: Theme.fontSizeHeader
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: backend.episodeModel.count + " episodes"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeSmall
                    visible: backend.episodeModel.count > 0
                }
            }

            // Loading spinner for episodes
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: backend.isLoading && backend.episodeModel.count === 0

                PortalSpinner {
                    anchors.centerIn: parent
                    width: Theme.isCompact ? 50 : 60
                    height: width
                    running: visible
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.verticalCenter
                    anchors.topMargin: 40
                    text: "Loading episodes..."
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeMedium
                }
            }

            // Episode list
            ListView {
                id: episodeListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0
                clip: true
                visible: backend.episodeModel.count > 0

                model: backend.episodeModel

                // Season grouping
                section.property: "season"
                section.criteria: ViewSection.FullString
                section.delegate: SeasonSectionHeader {
                    width: episodeListView.width
                    section: section
                    isCollapsed: window.collapsedSeasons[section] === true
                    onClicked: window.toggleSeason(section)
                }

                delegate: Item {
                    property bool isSeasonCollapsed: window.collapsedSeasons[model.season] === true

                    width: episodeListView.width
                    height: isSeasonCollapsed ? 0 : episodeDelegate.implicitHeight + Theme.spacingSmall
                    clip: true

                    Behavior on height {
                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                    }

                    EpisodeDelegate {
                        id: episodeDelegate
                        width: parent.width
                        opacity: parent.isSeasonCollapsed ? 0 : 1

                        Behavior on opacity {
                            NumberAnimation { duration: 150 }
                        }

                        episodeId: model.id
                        episodeCode: model.episodeCode
                        episodeName: model.name
                        airDate: model.airDate
                        characterCount: model.characterCount
                        isSelected: window.selectedEpisodeId === model.id

                        onClicked: {
                            window.selectedEpisodeId = model.id
                            backend.loadCharactersForEpisode(model.id)
                            // Auto-close drawer on compact mode
                            if (Theme.isCompact) {
                                drawerOpen = false
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    active: true
                    policy: ScrollBar.AsNeeded

                    contentItem: Rectangle {
                        implicitWidth: Theme.isCompact ? 4 : 6
                        radius: 3
                        color: Theme.portalGreen
                        opacity: parent.active ? 0.7 : 0.3
                        Behavior on opacity { NumberAnimation { duration: 200 } }
                    }

                    background: Rectangle {
                        implicitWidth: Theme.isCompact ? 4 : 6
                        radius: 3
                        color: Theme.spacePurple
                        opacity: 0.3
                    }
                }

                add: Transition {
                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
                    NumberAnimation { property: "scale"; from: 0.9; to: 1; duration: 200; easing.type: Easing.OutQuad }
                }
            }
        }
    }

    // Floating Hamburger Button
    HamburgerButton {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: Theme.spacingLarge
        z: 200
        visible: Theme.showHamburgerButton
        isOpen: drawerOpen
        onClicked: drawerOpen = !drawerOpen
    }

    // Character Grid Panel Component
    component CharacterGridPanel: Rectangle {
        property bool showHeader: true

        radius: Theme.radiusLarge
        color: Qt.rgba(Theme.spaceDark.r, Theme.spaceDark.g, Theme.spaceDark.b, 0.7)
        border.width: 1
        border.color: Qt.rgba(Theme.portalGreen.r, Theme.portalGreen.g, Theme.portalGreen.b, 0.2)

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Theme.spacingLarge
            anchors.topMargin: Theme.headerTopMargin
            spacing: Theme.spacingMedium

            // Header with selected episode
            RowLayout {
                Layout.fillWidth: true
                visible: showHeader

                Text {
                    text: "Characters"
                    color: Theme.textPrimary
                    font.family: Theme.fontTitle
                    font.pixelSize: Theme.fontSizeHeader
                }

                // Selected episode indicator
                Rectangle {
                    visible: backend.selectedEpisodeName !== "" && !Theme.isCompact
                    Layout.leftMargin: Theme.spacingMedium
                    implicitWidth: selectedEpText.implicitWidth + Theme.spacingLarge
                    implicitHeight: 28
                    radius: 14
                    color: Theme.spacePurple
                    border.width: 1
                    border.color: Theme.dimensionPurple

                    Text {
                        id: selectedEpText
                        anchors.centerIn: parent
                        text: backend.selectedEpisodeName
                        color: Theme.portalGreen
                        font.family: Theme.fontDisplay
                        font.pixelSize: Theme.fontSizeSmall
                    }
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: backend.characterModel.count + " characters"
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeMedium
                    visible: backend.characterModel.count > 0
                }
            }

            // Episode header banner with animated portal effect
            EpisodeHeaderBanner {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.spacingMedium
                Layout.rightMargin: Theme.spacingMedium
                episodeCode: {
                    var parts = backend.selectedEpisodeName.split(" - ")
                    return parts.length > 0 ? parts[0] : ""
                }
                episodeName: {
                    var parts = backend.selectedEpisodeName.split(" - ")
                    return parts.length > 1 ? parts.slice(1).join(" - ") : ""
                }
            }

            // Loading spinner for characters
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: backend.isLoading && window.selectedEpisodeId !== -1

                PortalSpinner {
                    anchors.centerIn: parent
                    width: Theme.isCompact ? 60 : 80
                    height: width
                    running: visible
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.verticalCenter
                    anchors.topMargin: Theme.isCompact ? 40 : 60
                    text: "Opening portal..."
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeMedium
                }
            }

            // Empty state - Random character showcase
            Item {
                id: emptyStateItem
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: !backend.isLoading && backend.characterModel.count === 0

                property bool showRandomCharacter: backend.cachedCharacterCount > 0 && backend.randomCharacter.id !== undefined

                Column {
                    anchors.centerIn: parent
                    spacing: Theme.spacingLarge

                    // Random character display (when cache has characters)
                    Item {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Theme.showcaseImageSize + 40
                        height: Theme.showcaseImageSize + 100
                        visible: emptyStateItem.showRandomCharacter
                        opacity: emptyStateItem.showRandomCharacter ? 1 : 0

                        Behavior on opacity {
                            NumberAnimation { duration: 300 }
                        }

                        // Portal frame for character
                        Rectangle {
                            id: showcaseFrame
                            anchors.top: parent.top
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: Theme.showcaseImageSize
                            height: Theme.showcaseImageSize
                            radius: width / 2
                            color: "transparent"
                            border.width: 3
                            border.color: Theme.portalGreen

                            SequentialAnimation on border.color {
                                running: emptyStateItem.showRandomCharacter
                                loops: Animation.Infinite
                                ColorAnimation { to: Theme.portalGreenGlow; duration: 1500; easing.type: Easing.InOutSine }
                                ColorAnimation { to: Theme.portalGreenDark; duration: 1500; easing.type: Easing.InOutSine }
                            }

                            Image {
                                id: showcaseImage
                                anchors.centerIn: parent
                                width: parent.width - 12
                                height: parent.height - 12
                                source: backend.randomCharacter.imageUrl || ""
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                                visible: false
                            }

                            ShaderEffect {
                                anchors.centerIn: parent
                                width: showcaseImage.width
                                height: showcaseImage.height
                                visible: showcaseImage.status === Image.Ready
                                property variant source: showcaseImage
                                fragmentShader: "qrc:/shaders/circlemask.frag.qsb"
                            }

                            // Placeholder while loading
                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width - 12
                                height: parent.height - 12
                                radius: width / 2
                                color: Theme.spacePurple
                                visible: showcaseImage.status !== Image.Ready

                                Text {
                                    anchors.centerIn: parent
                                    text: backend.randomCharacter.name ? backend.randomCharacter.name.charAt(0).toUpperCase() : "?"
                                    color: Theme.portalGreen
                                    font.family: Theme.fontTitle
                                    font.pixelSize: Theme.fontSizeHeader
                                }
                            }

                            // Status indicator
                            Rectangle {
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                anchors.margins: 8
                                width: 24
                                height: 24
                                radius: 12
                                color: backend.randomCharacter.status === "Alive" ? Theme.statusAlive :
                                       backend.randomCharacter.status === "Dead" ? Theme.statusDead :
                                       Theme.statusUnknown
                                border.width: 2
                                border.color: Theme.spaceDark

                                SequentialAnimation on scale {
                                    running: backend.randomCharacter.status === "Alive"
                                    loops: Animation.Infinite
                                    NumberAnimation { to: 1.2; duration: 800; easing.type: Easing.OutQuad }
                                    NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InQuad }
                                }
                            }
                        }

                        // Character name
                        Text {
                            anchors.top: showcaseFrame.bottom
                            anchors.topMargin: Theme.spacingMedium
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: backend.randomCharacter.name || ""
                            color: Theme.textPrimary
                            font.family: Theme.fontBody
                            font.pixelSize: Theme.fontSizeTitle
                            font.bold: true
                        }

                        // Species and status
                        Text {
                            anchors.top: showcaseFrame.bottom
                            anchors.topMargin: Theme.spacingMedium + 28
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: (backend.randomCharacter.status || "") + " \u2022 " + (backend.randomCharacter.species || "")
                            color: Theme.textSecondary
                            font.pixelSize: Theme.fontSizeMedium
                        }
                    }

                    // Original "?" state (when no cache)
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: Theme.isCompact ? 80 : 100
                        height: width
                        radius: width / 2
                        color: "transparent"
                        border.width: 3
                        border.color: Theme.portalGreen
                        opacity: 0.4
                        visible: !emptyStateItem.showRandomCharacter

                        Text {
                            anchors.centerIn: parent
                            text: "?"
                            color: Theme.portalGreen
                            font.family: Theme.fontTitle
                            font.pixelSize: Theme.isCompact ? 36 : 48
                            opacity: 0.6
                        }

                        SequentialAnimation on opacity {
                            running: !emptyStateItem.showRandomCharacter
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.6; duration: 1500; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 0.3; duration: 1500; easing.type: Easing.InOutSine }
                        }
                    }

                    // Shuffle button
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: shuffleText.implicitWidth + Theme.spacingXLarge * 2
                        height: 40
                        radius: 20
                        color: shuffleMouseArea.containsMouse ? Theme.spacePurple : "transparent"
                        border.width: 1
                        border.color: Theme.portalGreen
                        visible: emptyStateItem.showRandomCharacter

                        Behavior on color { ColorAnimation { duration: 150 } }

                        Text {
                            id: shuffleText
                            anchors.centerIn: parent
                            text: "Shuffle Character"
                            color: Theme.portalGreen
                            font.pixelSize: Theme.fontSizeMedium
                        }

                        MouseArea {
                            id: shuffleMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.shuffleRandomCharacter()
                        }
                    }

                    // Instruction text
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: emptyStateItem.showRandomCharacter
                            ? "Or select an episode to explore"
                            : (Theme.isCompact ? "Select an episode" : "Select an episode to view characters")
                        color: Theme.textMuted
                        font.pixelSize: Theme.fontSizeLarge
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Characters will appear through the portal"
                        color: Theme.textMuted
                        font.pixelSize: Theme.fontSizeMedium
                        opacity: 0.7
                        visible: !Theme.isCompact && !emptyStateItem.showRandomCharacter
                    }
                }
            }

            // Character grid - wrapper for proper centering within panel
            Item {
                id: gridContainer
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: !backend.isLoading && backend.characterModel.count > 0

                // Calculate centering based on container width (not GridView width)
                property int columns: Math.max(1, Math.floor(width / Theme.gridCellWidth))
                property int gridContentWidth: columns * Theme.gridCellWidth

                GridView {
                    id: characterGridView
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.gridContentWidth
                    height: parent.height
                    cellWidth: Theme.gridCellWidth
                    cellHeight: Theme.gridCellHeight
                    clip: true

                    model: backend.characterModel

                    delegate: CharacterCard {
                        width: Theme.cardWidth
                        height: Theme.cardHeight
                        onClicked: (charId, charName, charStatus, charSpecies, charType, charGender, charOrigin, charLocation, charImageUrl, charEpisodeCount, charCreated) => {
                            characterDetailPopup.show(charId, charName, charStatus, charSpecies, charType, charGender, charOrigin, charLocation, charImageUrl, charEpisodeCount, charCreated)
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        active: true
                        policy: ScrollBar.AsNeeded

                        contentItem: Rectangle {
                            implicitWidth: Theme.isCompact ? 4 : 6
                            radius: 3
                            color: Theme.portalGreen
                            opacity: parent.active ? 0.7 : 0.3
                            Behavior on opacity { NumberAnimation { duration: 200 } }
                        }

                        background: Rectangle {
                            implicitWidth: Theme.isCompact ? 4 : 6
                            radius: 3
                            color: Theme.spacePurple
                            opacity: 0.3
                        }
                    }
                }
            }
        }
    }

    // Error popup
    Rectangle {
        id: errorPopup
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        width: Math.min(errorText.implicitWidth + 40, window.width - 40)
        height: 50
        radius: 25
        color: Theme.statusDead
        opacity: 0
        visible: opacity > 0
        z: 300

        Text {
            id: errorText
            anchors.centerIn: parent
            width: parent.width - 20
            color: "white"
            font.pixelSize: Theme.fontSizeMedium
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }

        Behavior on opacity {
            NumberAnimation { duration: 300 }
        }
    }

    // Handle errors
    Connections {
        target: backend
        function onErrorOccurred(message) {
            errorText.text = message
            errorPopup.opacity = 1
            errorHideTimer.restart()
        }
    }

    Timer {
        id: errorHideTimer
        interval: 5000
        onTriggered: errorPopup.opacity = 0
    }

    // Character detail popup
    CharacterDetailPopup {
        id: characterDetailPopup
    }
}
