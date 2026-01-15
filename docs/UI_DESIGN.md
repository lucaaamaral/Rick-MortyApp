# Rick and Morty Viewer - UI Design Document

## Overview

This document defines the visual design, responsive behavior, component specifications, and animations for the Rick and Morty Viewer application.

---

## Visual Design

### Color Palette

The application uses colors inspired by the show's distinctive visual style.

| Category | Color | Hex | Usage |
|----------|-------|-----|-------|
| **Portal Green** | Neon Green | `#39FF14` | Primary accent, borders, highlights |
| | Lime Green | `#32CD32` | Hover states, secondary accent |
| | Matrix Green | `#00FF41` | Glow effects, active states |
| **Character** | Morty Yellow | `#FFE135` | Warning states, unknown status |
| | Rick Lab Coat | `#C8E6F5` | Light accent |
| **Background** | Space Black | `#0D0D0D` | Main background |
| | Space Dark | `#1A1A2E` | Card backgrounds, panels |
| | Space Purple | `#16213E` | Secondary backgrounds |
| **Accent** | Dimension Purple | `#9D4EDD` | Episode badges, highlights |
| | Meeseeks Blue | `#00B4D8` | Character count, links |
| **Status** | Alive | `#39FF14` | Green pulse indicator |
| | Dead | `#FF3131` | Red static indicator |
| | Unknown | `#FFE135` | Yellow static indicator |
| **Text** | Primary | `#FFFFFF` | Headings, names |
| | Secondary | `#B0B0B0` | Descriptions, metadata |
| | Muted | `#666666` | Hints, disabled text |

### Typography

| Size | Pixels | Usage |
|------|--------|-------|
| Small | 12px (11px compact) | Metadata, badges |
| Medium | 14px (13px compact) | Body text, descriptions |
| Large | 18px (16px compact) | Card titles, subtitles |
| Title | 24px (22px compact) | Section headers |
| Header | 32px (24px compact) | Page titles |

---

## Responsive Design

### Breakpoints

| Breakpoint | Width | Description |
|------------|-------|-------------|
| **Compact** | < 600px | Mobile phones, small tablets |
| **Medium** | 600-900px | Tablets, small laptops |
| **Wide** | 900-1200px | Laptops, desktops |
| **Extra Wide** | > 1200px | Large monitors |

### Layout Strategy - Floating Drawer

The application uses a floating drawer pattern for the episode list. The character grid is always the main content area.

```
COMPACT (< 600px)
┌─────────────────────────────────────────────────┐
│  ┌───┐                                          │
│  │ ☰ │  Hamburger button                        │
│  └───┘                                          │
│  ┌─────────────────────────────────────────┐    │
│  │  ┌─────────────────────────────────┐    │    │
│  │  │     Episode Name Badge          │    │    │  ← Top padding for selected episode
│  │  └─────────────────────────────────┘    │    │
│  │                                         │    │
│  │         Character Grid                  │    │
│  │        (full width, 1 column)           │    │
│  │                                         │    │
│  └─────────────────────────────────────────┘    │
│                                                  │
│  Drawer opens: 75% width overlay with backdrop   │
└─────────────────────────────────────────────────┘

MEDIUM/WIDE (600-1200px)
┌─────────────────────────────────────────────────────────────┐
│  ┌───┐                                                      │
│  │ ☰ │  Hamburger toggles drawer                            │
│  └───┘                                                      │
│  ┌────────────┐  ┌──────────────────────────────────────┐   │
│  │            │  │  Episode Name Badge                  │   │
│  │  Episodes  │  │                                      │   │
│  │  Drawer    │  │       Character Grid                 │   │
│  │            │  │    (auto-fit columns, max 320px)     │   │
│  │ (pushes    │  │                                      │   │
│  │  content)  │  │                                      │   │
│  └────────────┘  └──────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘

EXTRA WIDE (> 1200px)
┌─────────────────────────────────────────────────────────────┐
│  ┌────────────┐  ┌──────────────────────────────────────┐   │
│  │            │  │  Episode Name Badge                  │   │
│  │  Episodes  │  │                                      │   │
│  │  Drawer    │  │       Character Grid                 │   │
│  │            │  │    (auto-fit, cards max 320px)       │   │
│  │ (always    │  │                                      │   │
│  │  visible)  │  │    No hamburger button               │   │
│  └────────────┘  └──────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### Drawer Behavior Matrix

| Property | Compact | Medium | Wide | Extra Wide |
|----------|---------|--------|------|------------|
| **Default State** | Hidden | Visible | Visible | Always Visible |
| **Width** | 75% window | 300px | 320px | 360px |
| **Z-Index** | 100 (overlay) | 0 (inline) | 0 | 0 |
| **Hamburger** | Visible | Visible | Visible | Hidden |
| **On Episode Select** | Auto-close | Stay open | Stay open | Stay open |
| **Backdrop** | Yes (dim) | No | No | No |

### Drawer Width Rationale

**75% width for compact** (vs 67% or 85%):
- 67% feels too narrow on small phones - episode titles get truncated excessively
- 85% leaves barely any visible content behind - loses context
- 75% provides optimal balance: readable episode list while maintaining spatial awareness

---

## Component Specifications

### Episode Drawer

| Property | Compact | Medium | Wide | Extra Wide |
|----------|---------|--------|------|------------|
| Width | 75% window | 300px | 320px | 360px |
| Background | `spaceDark` @ 95% | `spaceDark` @ 95% | `spaceDark` @ 95% | `spaceDark` @ 95% |
| Border | 1px `portalGreen` @ 30% | 1px `portalGreen` @ 30% | 1px `portalGreen` @ 30% | 1px `portalGreen` @ 30% |
| Top Padding | 60px (hamburger clearance) | 60px | 60px | 16px |
| Internal Padding | 12px | 16px | 16px | 16px |

### Character Grid

| Property | Compact | Medium | Wide | Extra Wide |
|----------|---------|--------|------|------------|
| Cell Width | 100% - 48px | 240px | 260px | 280px |
| **Max Card Width** | - | **320px** | **320px** | **320px** |
| Columns | 1 | Auto-fit | Auto-fit | Auto-fit |
| Gap | 8px | 12px | 12px | 16px |
| Centering | Yes | Yes | Yes | Yes |

**Why max 320px for cards?**
- Prevents cards from becoming too wide on ultra-wide displays
- Maintains readable proportions for card content
- Ensures consistent visual hierarchy across screen sizes
- Cards wider than 320px start to feel "stretched" and lose their card-like feel

### Character Card

| Property | Compact | Medium | Wide | Extra Wide |
|----------|---------|--------|------|------------|
| Width | windowWidth - 48px | 240px | 260px | 280px |
| **Max Width** | - | **320px** | **320px** | **320px** |
| Height | 300px | 320px | 340px | 360px |
| Image Size | 120px | 140px | 148px | 160px |
| Border Radius | 16px | 16px | 16px | 16px |
| Image Border | 2px | 2px | 3px | 3px |

### Episode Name Header

The selected episode name displays as a badge in the character grid header with proper top padding.

```
┌─────────────────────────────────────────────────────┐
│                                                      │
│  ← Top padding: 60px (when hamburger visible)       │
│     or 16px (when hamburger hidden)                 │
│                                                      │
│  Characters          ┌─────────────────┐   5 chars  │
│                      │   S01E04        │            │
│                      └─────────────────┘            │
│                                                      │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐             │
│  │  Card   │  │  Card   │  │  Card   │             │
└─────────────────────────────────────────────────────┘
```

| Property | Value |
|----------|-------|
| Badge Background | `spacePurple` |
| Badge Border | 1px `dimensionPurple` |
| Badge Radius | 14px |
| Badge Height | 28px |
| Badge Text Color | `portalGreen` |
| Badge Text Size | `fontSizeSmall` |
| Header Top Margin | 60px (with hamburger) / 16px (without) |

### Hamburger Button

| Property | Value |
|----------|-------|
| Size | 44x44px (touch-friendly) |
| Position | Top-left, 16px margin |
| Z-Index | 200 (always on top) |
| Background | `spaceDark` @ 90% |
| Border | 1px `portalGreen` @ 30% |
| Bar Color | `portalGreen` |
| Bar Dimensions | 20px × 2px |
| Bar Spacing | 5px |
| Animation Duration | 200ms |
| Visibility | Hidden on Extra Wide |

---

## Character Detail Modal

When a character card is clicked, a modal appears with detailed information.

```
┌──────────────────────────────────────────────────────────┐
│                                                          │
│     ┌─────────────────────────────────────────────┐      │
│     │  ╳                                          │      │
│     │                                             │      │
│     │           ┌──────────────┐                  │      │
│     │           │              │                  │      │
│     │           │    Image     │                  │      │
│     │           │   (200px)    │                  │      │
│     │           │              │                  │      │
│     │           └──────────────┘                  │      │
│     │                                             │      │
│     │         Character Name                      │      │
│     │         ━━━━━━━━━━━━━━━                    │      │
│     │                                             │      │
│     │   Status    ●  Alive                        │      │
│     │   Species      Human                        │      │
│     │   Gender       Male                         │      │
│     │   Origin       Earth (C-137)                │      │
│     │   Location     Citadel of Ricks             │      │
│     │   Episodes     51                           │      │
│     │                                             │      │
│     └─────────────────────────────────────────────┘      │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

| Property | Compact | Desktop |
|----------|---------|---------|
| Modal Width | 90% window | 400px max |
| Modal Height | Auto | Auto, max 80% viewport |
| Image Size | 150px | 200px |
| Background | `spaceDark` | `spaceDark` |
| Border | 2px `portalGreen` | 2px `portalGreen` |
| Border Radius | 20px | 20px |
| Backdrop | `#000000` @ 70% | `#000000` @ 70% |
| Animation | Scale 0.9 → 1, fade in | Scale 0.9 → 1, fade in |

### Detail Rows

| Property | Value |
|----------|-------|
| Label Color | `textSecondary` |
| Value Color | `textPrimary` |
| Row Spacing | 12px |
| Section Divider | 1px `portalGreen` @ 20% |

---

## Random Character Showcase (Empty State)

When no episode is selected, show a featured/random character to make the UI more engaging.

```
┌─────────────────────────────────────────────────────────┐
│                                                          │
│           ✦ Featured Character ✦                        │
│                                                          │
│              ┌──────────────┐                            │
│              │              │                            │
│              │    Image     │   ← Larger image (180px)   │
│              │              │                            │
│              └──────────────┘                            │
│                                                          │
│                Rick Sanchez                              │
│             Alive • Human • Male                         │
│                                                          │
│        "Select an episode to explore more"               │
│                                                          │
│              [ Shuffle Character ]                       │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

### Showcase Behavior

| Property | Value |
|----------|-------|
| Trigger | No episode selected AND characters loaded in cache |
| Character Selection | Random from all cached characters |
| Shuffle Button | Selects new random character |
| Auto-shuffle | Every 30 seconds (optional) |
| Animation | Fade transition on shuffle (300ms) |

### Showcase Components

| Element | Compact | Desktop |
|---------|---------|---------|
| Portal Frame | 120px | 150px |
| Image Size | 108px | 138px |
| Name Font | 18px bold | 20px bold |
| Info Font | 14px | 16px |
| Button Height | 36px | 40px |
| Button Width | auto (padding 24px) | auto (padding 32px) |

### Shuffle Button

| Property | Value |
|----------|-------|
| Background | transparent (hover: `spacePurple`) |
| Border | 1px `portalGreen` |
| Border Radius | 20px |
| Text Color | `portalGreen` |
| Hover Transition | 150ms |

---

## Season Grouping

Episodes in the drawer are grouped by season with collapsible section headers.

```
┌────────────────────────────────────────┐
│  Episodes                    51 eps    │
├────────────────────────────────────────┤
│  ┌────────────────────────────────┐    │
│  │ [S01]  Season 1    11 episodes │    │  ← Section Header
│  └────────────────────────────────┘    │
│    ┌──────────────────────────────┐    │
│    │ S01E01  Pilot         19 ♦  │    │  ← Episode Delegate
│    └──────────────────────────────┘    │
│    ┌──────────────────────────────┐    │
│    │ S01E02  Lawnmower Dog 14 ♦  │    │
│    └──────────────────────────────┘    │
│    ...                                  │
│  ┌────────────────────────────────┐    │
│  │ [S02]  Season 2    10 episodes │    │
│  └────────────────────────────────┘    │
│    ...                                  │
└────────────────────────────────────────┘
```

### Section Header Specifications

| Property | Compact | Desktop |
|---------|---------|---------|
| Height | 36px | 44px |
| Background | `spacePurple` @ 50% |  `spacePurple` @ 50% |
| Border Radius | 8px | 8px |
| Bottom Border | 1px `portalGreen` @ 30% | 1px `portalGreen` @ 30% |

### Section Header Content

| Element | Property | Value |
|---------|----------|-------|
| Season Badge | Width | 48px |
| | Height | 24px |
| | Background | `dimensionPurple` |
| | Text | "S01", "S02", etc. (monospace) |
| | Text Color | `textPrimary` |
| Season Title | Font | 14px (compact) / 16px (desktop), bold |
| | Color | `textPrimary` |
| Episode Count | Font | `fontSizeSmall` |
| | Color | `textSecondary` |

### Theme Properties for Sections

```qml
readonly property int sectionHeaderHeight: isCompact ? 36 : 44
readonly property int sectionFontSize: isCompact ? 14 : 16
```

---

## Episode Header Banner

When an episode is selected, an animated banner displays at the top of the character grid showing the episode title with a portal/flame effect.

```
┌────────────────────────────────────────────────────────────────────────┐
│                                                                         │
│   ○○○                                                                   │
│  ○   ○   [S01E01]  Pilot                                               │
│   ○○○      ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑                          │
│  rotating   flame particles rising upward                               │
│   rings                                                                 │
│  ════════════════════════════════════════════════════════════════════  │
│              ↑ pulsing gradient line                                    │
└────────────────────────────────────────────────────────────────────────┘
```

### Banner Specifications

| Property | Compact | Desktop |
|----------|---------|---------|
| Height | 60px | 80px |
| Background | `spaceDark` @ 90% | `spaceDark` @ 90% |
| Border | 2px `portalGreen` | 2px `portalGreen` |
| Border Radius | 12px | 12px |
| Clip | true | true |

### Banner Content

| Element | Property | Compact | Desktop |
|---------|----------|---------|---------|
| Episode Badge | Width | 60px | 80px |
| | Height | 32px | 40px |
| | Background | `spacePurple` | `spacePurple` |
| | Border | 2px `dimensionPurple` | 2px `dimensionPurple` |
| | Font | 14px monospace bold | 18px monospace bold |
| | Text Color | `portalGreen` | `portalGreen` |
| Episode Title | Font | 18px bold | 24px bold |
| | Color | `textPrimary` | `textPrimary` |
| | Elide | Right | Right |

### Animation Layers (back to front)

1. **Base Gradient**
   - Horizontal gradient: transparent → `portalGreen` @ 10% → transparent
   - Static layer

2. **Rotating Outer Ring**
   - Position: left side, extending 30% beyond edge
   - Size: 2× banner height
   - Border: 3px `portalGreen` @ 40%
   - Arc highlight: `portalGreenGlow` on right edge
   - Animation: 360° rotation, 4000ms, infinite

3. **Counter-Rotating Middle Ring**
   - Position: left side, extending 20% beyond edge
   - Size: 1.5× banner height
   - Border: 2px `portalGreenDark` @ 30%
   - Animation: -360° rotation, 3000ms, infinite

4. **Flame Particles** (12 particles)
   - Random X positions across banner width
   - Size: 4-8px circular
   - Color: `portalGreen`
   - Animation sequence:
     - Staggered start: random 0-2000ms delay
     - Y motion: bottom → top (1500-2500ms, OutQuad)
     - X drift: ±20px random horizontal movement
     - Opacity: 0 → 0.8 (300ms) → 0 (remaining time, InQuad)

5. **Color Cycling Overlay**
   - Full banner rectangle @ 15% opacity
   - Color animation: `portalGreen` → `portalGreenGlow` → `portalGreenDark`
   - Duration: 3000ms total cycle, infinite

6. **Bottom Accent Line**
   - Height: 2px
   - Horizontal gradient: transparent → `portalGreen` → transparent
   - Opacity animation: 0.5 → 1.0 → 0.5 (2000ms, InOutSine)

### Theme Properties for Banner

```qml
readonly property int bannerHeight: isCompact ? 60 : 80
readonly property int bannerTitleSize: isCompact ? 18 : 24
```

---

## Character Grid Centering

The character grid should be horizontally centered within its container, ensuring cards don't awkwardly align to the left when there are fewer cards than columns.

### Centering Strategy

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                  │
│        ┌─────────┐  ┌─────────┐  ┌─────────┐                   │
│        │  Card   │  │  Card   │  │  Card   │                   │
│        └─────────┘  └─────────┘  └─────────┘                   │
│        ← contentPadding →     ← contentPadding →               │
│                                                                  │
│        Grid content is centered within available width          │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Implementation

The GridView uses calculated margins to center content:

```qml
GridView {
    property int columns: Math.max(1, Math.floor(width / cellWidth))
    property real contentPadding: Math.max(0, (width - (columns * cellWidth)) / 2)

    leftMargin: contentPadding
    rightMargin: contentPadding
}
```

### Behavior

| Scenario | Result |
|----------|--------|
| Grid width fits 3 columns exactly | No extra padding, cards fill width |
| Grid width fits 3.5 columns | Padding added to center 3 columns |
| Single column (compact) | Card centered horizontally |
| Many columns (extra wide) | Cards centered, max width respected |

---

## Animations

### Standard Animations

| Animation | Duration | Easing | Usage |
|-----------|----------|--------|-------|
| Drawer slide | 250ms | OutCubic | Drawer open/close |
| Content push | 250ms | OutCubic | Grid margin adjustment |
| Hamburger morph | 200ms | InOutQuad | Bar rotation/fade |
| Backdrop fade | 200ms | Linear | Dim overlay |
| Card reveal | 350ms | OutBack | Character card entrance |
| Portal pulse | 800ms × 2 | OutQuad/InQuad | Status indicator |
| Border glow | 1500ms × 2 | InOutSine | Card frame animation |
| Modal enter | 200ms | OutQuad | Scale + fade |
| Modal exit | 150ms | InQuad | Scale + fade |
| Hover glow | 200ms | Linear | Card hover state |

### Card Reveal Animation

Characters appear with a staggered "portal reveal" effect:

1. Card starts at scale 0.8, opacity 0
2. Staggered delay: 50ms + random(0-150ms)
3. Portal circle expands from center (green glow)
4. Card fades in and scales to 1.0
5. Portal glow fades out

### Loading States

| State | Animation |
|-------|-----------|
| Episode loading | Portal spinner with pulsing core |
| Character loading | "Opening portal..." text + spinner |
| Image loading | Letter placeholder (first initial) |
| Shuffle | Fade out → fade in (150ms each) |

---

## Interaction Patterns

### Touch Targets

All interactive elements meet minimum 44×44px touch target size:
- Hamburger button: 44×44px
- Episode delegates: Full width × 80px
- Character cards: Full card area
- Modal close button: 44×44px
- Shuffle button: 44×44px minimum

### Gestures (Mobile)

| Gesture | Action |
|---------|--------|
| Tap hamburger | Toggle drawer |
| Tap backdrop | Close drawer |
| Tap episode | Select episode, close drawer (compact) |
| Tap card | Open character detail modal |
| Swipe drawer edge | Open drawer (future enhancement) |

### Keyboard Navigation (Future)

| Key | Action |
|-----|--------|
| `Escape` | Close drawer/modal |
| `Enter` | Select focused item |
| `Arrow keys` | Navigate grid/list |
| `Tab` | Focus next element |

---

## Scrollbars

### Episode List Scrollbar

| Property | Value |
|----------|-------|
| Width | 4px (compact) / 6px (desktop) |
| Color | `portalGreen` |
| Opacity | 70% (active) / 30% (inactive) |
| Track Color | `spacePurple` @ 30% |
| Border Radius | 3px |

### Character Grid Scrollbar

Same as episode list scrollbar.

---

## Theme QML Properties

```qml
pragma Singleton
import QtQuick

QtObject {
    // Window dimensions (updated from Main.qml)
    property real windowWidth: 1280
    property real windowHeight: 800

    // Breakpoint detection
    readonly property bool isCompact: windowWidth < 600
    readonly property bool isMedium: windowWidth >= 600 && windowWidth < 900
    readonly property bool isWide: windowWidth >= 900 && windowWidth < 1200
    readonly property bool isExtraWide: windowWidth >= 1200

    // Drawer configuration
    readonly property real drawerWidth: {
        if (isCompact) return windowWidth * 0.75  // 75% for better balance
        if (isMedium) return 300
        if (isWide) return 320
        return 360
    }
    readonly property bool showHamburgerButton: !isExtraWide
    readonly property bool drawerDefaultOpen: !isCompact

    // Character card sizing
    readonly property real cardWidth: {
        if (isCompact) return windowWidth - 48
        if (isMedium) return Math.min(240, 320)
        if (isWide) return Math.min(260, 320)
        return Math.min(280, 320)
    }
    readonly property real cardMaxWidth: 320  // Maximum width for any screen

    readonly property real cardHeight: {
        if (isCompact) return 300
        if (isMedium) return 320
        if (isWide) return 340
        return 360
    }

    // Header top margin (clearance for hamburger)
    readonly property real headerTopMargin: showHamburgerButton ? 60 : spacingLarge

    // ... (other existing properties)
}
```

---

## UI Improvement Recommendations

### High Priority

1. **Character Detail Modal** - Clicking a card should show more info (origin, location, episode count, created date). Currently cards only show basic info.

2. **Random Character Showcase** - Empty state showing "?" is uninspiring. Show a random character with "Select an episode to explore more" message.

3. **Loading Skeleton** - Replace plain spinners with skeleton loading cards that match card shape. Provides better perceived performance.

4. **Search/Filter** - Add search box in drawer to filter episodes by name or code. Add filter in grid to search characters by name.

### Medium Priority

5. **Season Grouping** - Group episodes by season with collapsible sections (S01, S02, etc.). Makes navigation easier with 51 episodes.

6. **Character Status Filter** - Quick filter buttons in grid header: "All" / "Alive" / "Dead" / "Unknown".

7. **Favorites System** - Heart icon on cards to mark favorites. Separate "Favorites" section in drawer.

8. **Swipe Gestures** - Swipe from left edge to open drawer on mobile. More natural mobile UX.

### Lower Priority

9. **Episode Info Tooltip** - Long-press episode to see air date and character count without selecting.

10. **Share Character** - Share button in detail modal to copy character link or image.

11. **Dark/Light Theme Toggle** - Some users prefer light themes (though dark fits the show aesthetic).

12. **Reduce Motion Option** - Accessibility setting to disable/reduce animations for users with motion sensitivity.

### Performance Optimizations

13. **Image Lazy Loading** - Only load images for visible cards + 1 row buffer.

14. **Virtual Scrolling** - For the 800+ character dataset, use virtual list rendering.

15. **Image Caching** - Cache character images locally after first load.

---

## Implementation Checklist

### Layout & Responsiveness
- [x] Drawer slides from left
- [x] Hamburger button toggles drawer
- [x] Content pushes aside on medium/wide
- [x] Overlay mode on compact
- [x] Auto-close on episode select (compact)
- [x] Drawer width updated to 75% on compact
- [x] Card max width of 320px implemented
- [x] Header top padding for episode badge
- [ ] Character grid horizontally centered

### Components
- [x] HamburgerButton with animation
- [x] EpisodeDelegate with hover effects
- [x] CharacterCard with portal reveal
- [x] PortalSpinner for loading
- [x] CharacterDetailPopup
- [ ] SeasonSectionHeader
- [ ] EpisodeHeaderBanner (animated)
- [ ] RandomCharacterShowcase
- [ ] LoadingSkeleton cards

### Animations
- [x] Drawer slide animation
- [x] Hamburger morph animation
- [x] Card reveal animation
- [x] Status pulse animation
- [x] Modal enter/exit animations
- [ ] Banner portal/flame effect
- [ ] Shuffle transition

### Polish
- [ ] Search/filter functionality
- [ ] Season grouping in episode drawer
- [ ] Episode header banner with animation
- [ ] Random character showcase (empty state)
- [ ] Favorites system
- [ ] Swipe gestures
