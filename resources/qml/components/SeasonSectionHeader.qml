import QtQuick
import QtQuick.Layouts
import ".."

Rectangle {
    id: root
    height: Theme.sectionHeaderHeight
    color: Qt.rgba(Theme.spaceDark.r, Theme.spaceDark.g, Theme.spaceDark.b, 0.95)
    radius: Theme.radiusMedium
    border.width: 1
    border.color: Qt.rgba(1, 1, 1, 0.15)

    required property string section
    property int seasonNumber: parseInt(section) || 0
    property bool isCollapsed: false

    signal clicked()

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onClicked: root.clicked()
    }

    // Hover effect
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: Theme.portalGreen
        opacity: mouseArea.containsMouse ? 0.1 : 0
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacingMedium
        anchors.rightMargin: Theme.spacingMedium
        spacing: Theme.spacingSmall

        // Season badge
        Rectangle {
            Layout.preferredWidth: 48
            Layout.preferredHeight: 24
            radius: Theme.radiusSmall
            color: Theme.dimensionPurple

            Text {
                anchors.centerIn: parent
                text: "S" + (root.seasonNumber < 10 ? "0" : "") + root.seasonNumber
                color: Theme.textPrimary
                font.pixelSize: Theme.fontSizeMedium
                font.bold: true
                font.family: "monospace"
            }
        }

        // Season title
        Text {
            text: "Season " + root.seasonNumber
            color: Theme.textPrimary
            font.pixelSize: Theme.sectionFontSize
            font.bold: true
        }

        Item { Layout.fillWidth: true }

        // Collapse indicator
        Text {
            text: root.isCollapsed ? "+" : "-"
            color: Theme.textSecondary
            font.pixelSize: Theme.fontSizeMedium
            font.bold: true
        }
    }
}
