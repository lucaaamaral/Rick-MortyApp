import QtQuick
import QtQuick.Layouts
import ".."

Rectangle {
    id: root
    implicitHeight: 72
    radius: Theme.radiusMedium
    color: mouseArea.containsMouse ? Qt.lighter(Theme.spaceDark, 1.4) : Theme.spaceDark
    border.width: mouseArea.containsMouse || isSelected ? 2 : 1
    border.color: mouseArea.containsMouse || isSelected ? Theme.portalGreen : Qt.rgba(1, 1, 1, 0.1)

    property int episodeId
    property string episodeCode
    property string episodeName
    property string airDate
    property int characterCount
    property bool isSelected: false

    Behavior on color { ColorAnimation { duration: 150 } }
    Behavior on border.color { ColorAnimation { duration: 150 } }
    Behavior on border.width { NumberAnimation { duration: 150 } }

    // Glow effect when selected
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.width: 4
        border.color: Theme.portalGreen
        opacity: isSelected ? 0.3 : 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingMedium

        // Episode code badge
        Rectangle {
            Layout.preferredWidth: 65
            Layout.preferredHeight: 44
            radius: Theme.radiusSmall
            color: Theme.spacePurple
            border.width: 1
            border.color: Theme.dimensionPurple

            Text {
                anchors.centerIn: parent
                text: root.episodeCode
                color: Theme.portalGreen
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
                font.family: "monospace"
            }
        }

        // Episode info
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                Layout.fillWidth: true
                text: root.episodeName
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                text: root.airDate
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeSmall
            }
        }

        // Character count
        RowLayout {
            spacing: 4

            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: Theme.meeseeksBlue
                opacity: 0.3

                Text {
                    anchors.centerIn: parent
                    text: "\u{1F464}"
                    font.pixelSize: 10
                }
            }

            Text {
                text: root.characterCount.toString()
                color: Theme.meeseeksBlue
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
            }
        }

        // Arrow indicator
        Text {
            text: "›"
            color: mouseArea.containsMouse ? Theme.portalGreen : Theme.textMuted
            font.pixelSize: Theme.fontSizeTitle
            font.bold: true

            Behavior on color { ColorAnimation { duration: 150 } }

            transform: Translate {
                x: mouseArea.containsMouse ? 3 : 0
                Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    signal clicked()
}
