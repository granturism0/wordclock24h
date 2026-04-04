#!/bin/sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_DIR="$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)"
BUNDLE_SCRIPT="$PROJECT_DIR/tools/build-app-bundle.py"
BUNDLE_FILE="$PROJECT_DIR/data/app-bundle.txt"

python3 "$BUNDLE_SCRIPT"

echo
echo "WordClock App bundle ready:"
echo "  $BUNDLE_FILE"
echo
echo "Next step:"
echo "  Upload app-bundle.txt to your update server path"
echo "  or install it manually via /fs -> WordClock App bundle (/app)"
