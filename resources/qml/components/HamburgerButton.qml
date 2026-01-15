import QtQuick
import ".."

Item {
    id: root
    width: 44
    height: 44

    property bool isOpen: false
    signal clicked()

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusMedium
        color: Qt.rgba(Theme.spaceDark.r, Theme.spaceDark.g, Theme.spaceDark.b, 0.9)
        border.width: 1
        border.color: Qt.rgba(Theme.portalGreen.r, Theme.portalGreen.g, Theme.portalGreen.b, 0.3)

        // Hover glow
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            radius: parent.radius + 2
            color: "transparent"
            border.width: 4
            border.color: Theme.portalGreen
            opacity: mouseArea.containsMouse ? 0.3 : 0
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }

        // Three lines container
        Item {
            anchors.centerIn: parent
            width: 20
            height: 14

            // Top line
            Rectangle {
                id: topLine
                width: parent.width
                height: 2
                radius: 1
                color: Theme.portalGreen
                y: root.isOpen ? 6 : 0
                rotation: root.isOpen ? 45 : 0
                transformOrigin: Item.Center

                Behavior on y { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
                Behavior on rotation { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
            }

            // Middle line
            Rectangle {
                id: middleLine
                width: parent.width
                height: 2
                radius: 1
                color: Theme.portalGreen
                y: 6
                opacity: root.isOpen ? 0 : 1

                Behavior on opacity { NumberAnimation { duration: 150 } }
            }

            // Bottom line
            Rectangle {
                id: bottomLine
                width: parent.width
                height: 2
                radius: 1
                color: Theme.portalGreen
                y: root.isOpen ? 6 : 12
                rotation: root.isOpen ? -45 : 0
                transformOrigin: Item.Center

                Behavior on y { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
                Behavior on rotation { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: root.clicked()
        }
    }

    // Scale feedback on press
    scale: mouseArea.pressed ? 0.9 : 1.0
    Behavior on scale { NumberAnimation { duration: 100 } }
}
