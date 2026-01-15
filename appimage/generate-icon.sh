#!/bin/bash
# Generate a placeholder icon for the AppImage
# Requires ImageMagick (convert command)

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
OUTPUT="$SCRIPT_DIR/icon.png"

if command -v convert &> /dev/null; then
    # Create a simple colored square with text
    convert -size 256x256 xc:'#00b4c5' \
        -fill white \
        -font DejaVu-Sans-Bold \
        -pointsize 48 \
        -gravity center \
        -annotate 0 "R&M\nViewer" \
        "$OUTPUT"
    echo "Created icon at $OUTPUT"
else
    echo "ImageMagick not installed. Please create a 256x256 PNG icon manually at:"
    echo "  $OUTPUT"
    echo ""
    echo "Or install ImageMagick and run this script again:"
    echo "  sudo apt install imagemagick"
fi
