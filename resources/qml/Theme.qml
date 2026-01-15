pragma Singleton
import QtQuick

QtObject {
    // Window dimensions (updated from Main.qml)
    property real windowWidth: 1280
    property real windowHeight: 800

    // Breakpoint detection
    readonly property bool isCompact: windowWidth < 768
    readonly property bool isMedium: windowWidth >= 768 && windowWidth < 1024
    readonly property bool isWide: windowWidth >= 1024 && windowWidth < 1280
    readonly property bool isExtraWide: windowWidth >= 1280

    // Layout mode
    readonly property bool useSingleColumn: isCompact

    // Portal Green palette
    readonly property color portalGreen: "#39FF14"
    readonly property color portalGreenDark: "#32CD32"
    readonly property color portalGreenGlow: "#00FF41"

    // Character colors
    readonly property color mortyYellow: "#FFE135"
    readonly property color rickLabCoat: "#C8E6F5"

    // Background
    readonly property color spaceBlack: "#0D0D0D"
    readonly property color spaceDark: "#1A1A2E"
    readonly property color spacePurple: "#16213E"

    // Accents
    readonly property color dimensionPurple: "#9D4EDD"
    readonly property color meeseeksBlue: "#00B4D8"

    // Status
    readonly property color statusAlive: "#39FF14"
    readonly property color statusDead: "#FF3131"
    readonly property color statusUnknown: "#FFE135"

    // Text
    readonly property color textPrimary: "#FFFFFF"
    readonly property color textSecondary: "#B0B0B0"
    readonly property color textMuted: "#666666"

    // Font families - Rick and Morty themed
    readonly property string fontTitle: "Bangers"      // Comic book style for main titles
    readonly property string fontDisplay: "Creepster"  // Horror style for episode codes
    readonly property string fontBody: "Nunito"        // Rounded, readable body text
    readonly property string fontBodyBold: "Nunito"    // Nunito bold for emphasis (use with font.bold: true)

    // Fonts - responsive scaling
    readonly property real fontScale: isCompact ? 0.9 : 1.0
    readonly property int fontSizeSmall: Math.round(12 * fontScale)
    readonly property int fontSizeMedium: Math.round(14 * fontScale)
    readonly property int fontSizeLarge: Math.round(18 * fontScale)
    readonly property int fontSizeTitle: Math.round(24 * fontScale)
    readonly property int fontSizeHeader: Math.round(isCompact ? 24 : 32)

    // Spacing - responsive
    readonly property int spacingSmall: isCompact ? 4 : 8
    readonly property int spacingMedium: isCompact ? 8 : 12
    readonly property int spacingLarge: isCompact ? 12 : 16
    readonly property int spacingXLarge: isCompact ? 16 : 24

    // Radius
    readonly property int radiusSmall: 8
    readonly property int radiusMedium: 12
    readonly property int radiusLarge: 16

    // Episode panel width - responsive
    readonly property real episodePanelWidth: {
        if (isCompact) return windowWidth - 32
        if (isMedium) return 280
        if (isWide) return 320
        return 360
    }

    // Character card size constraints
    readonly property real cardMinWidth: 160
    readonly property real cardMaxWidth: 240
    readonly property real cardMinHeight: 230
    readonly property real cardMaxHeight: 320

    // Character card dimensions - smooth scaling with window size
    readonly property real cardWidth: {
        if (isCompact) {
            // Ensure 2 cards fit side by side on compact
            return Math.min((windowWidth - 48) / 2 - spacingMedium, cardMaxWidth)
        }
        // Scale from min to max based on window width (768px -> 1280px range)
        var scale = Math.max(0, Math.min(1, (windowWidth - 768) / 512))
        return Math.round(cardMinWidth + scale * (cardMaxWidth - cardMinWidth))
    }

    readonly property real cardHeight: {
        // Scale proportionally with cardWidth
        var widthRatio = (cardWidth - cardMinWidth) / (cardMaxWidth - cardMinWidth)
        return Math.round(cardMinHeight + widthRatio * (cardMaxHeight - cardMinHeight))
    }

    readonly property real cardImageSize: {
        // Scale image size proportionally (80px min, 160px max)
        var widthRatio = (cardWidth - cardMinWidth) / (cardMaxWidth - cardMinWidth)
        return Math.round(80 + widthRatio * 80)
    }

    // Grid cell dimensions (card + spacing)
    readonly property real gridCellWidth: cardWidth + spacingMedium
    readonly property real gridCellHeight: cardHeight + spacingMedium

    // Drawer behavior
    readonly property real drawerWidth: {
        if (isCompact) return windowWidth * 0.75  // 75% - optimal balance for readability
        if (isMedium) return 300
        if (isWide) return 320
        return 360
    }
    readonly property bool showHamburgerButton: !isExtraWide
    readonly property bool drawerDefaultOpen: !isCompact

    // Header top margin (clearance for hamburger button)
    readonly property real headerTopMargin: showHamburgerButton ? 60 : spacingLarge

    // Season section header
    readonly property int sectionHeaderHeight: isCompact ? 36 : 44
    readonly property int sectionFontSize: isCompact ? 14 : 16

    // Episode header banner
    readonly property int bannerHeight: isCompact ? 60 : 80
    readonly property int bannerTitleSize: isCompact ? 18 : 24

    // Random character showcase
    readonly property int showcaseImageSize: isCompact ? 120 : 150
}
