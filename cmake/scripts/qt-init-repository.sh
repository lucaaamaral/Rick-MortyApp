#!/bin/sh
# Qt repository initialization script for Windows cross-compilation
# This script:
# 1. Initializes required Qt submodules with shallow clone
# 2. Applies MinGW compatibility patches

set -e

echo "=== Qt Repository Initialization ==="
echo "Working directory: $(pwd)"
echo "Patches to apply: $*"

# Initialize only needed submodules with shallow clone (much faster)
echo ""
echo "=== Initializing Qt submodules ==="
git submodule update --init --depth 1 --single-branch -- qtbase qtdeclarative qtshadertools qtsvg

# Apply patches to qtbase
if [ $# -gt 0 ]; then
    echo ""
    echo "=== Applying patches to qtbase ==="
    cd qtbase

    for patch_file in "$@"; do
        if [ -f "$patch_file" ]; then
            echo "Applying: $(basename "$patch_file")"
            # Use git apply with --check first to verify, then apply
            # --whitespace=nowarn to ignore whitespace warnings
            if git apply --check --whitespace=nowarn "$patch_file" 2>/dev/null; then
                git apply --whitespace=nowarn "$patch_file"
                echo "  Success"
            else
                echo "  Patch already applied or doesn't apply cleanly, skipping"
            fi
        else
            echo "Warning: Patch file not found: $patch_file"
        fi
    done

    cd ..
fi

echo ""
echo "=== Qt repository initialization complete ==="
