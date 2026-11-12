#!/bin/sh
# Installs the harmonograph camera display as a per-user XDG autostart entry,
# so it starts automatically after the desktop session loads.
#
# Usage:
#   ./install_autostart.sh            # runs drawCAM_cv2.py (default)
#   ./install_autostart.sh drawCAM_720.py
#
# Remove again with:
#   rm ~/.config/autostart/harmonograph-camera.desktop
set -e

SCRIPT_NAME="${1:-drawCAM_cv2.py}"
DIR="$(cd "$(dirname "$0")" && pwd)"
TARGET="$DIR/$SCRIPT_NAME"

if [ ! -f "$TARGET" ]; then
    echo "ERROR: $TARGET not found" >&2
    exit 1
fi

DEST="$HOME/.config/autostart/harmonograph-camera.desktop"
mkdir -p "$HOME/.config/autostart"

# sleep 3 lets the desktop settle before the camera window grabs the screen
cat > "$DEST" <<EOF
[Desktop Entry]
Type=Application
Name=Harmonograph Camera
Comment=Harmonograph webcam display ($SCRIPT_NAME)
Exec=sh -c 'sleep 3; exec env DISPLAY=:0 python3 "$TARGET"'
Terminal=false
X-GNOME-Autostart-enabled=true
EOF
chmod +x "$DEST"

echo "Installed: $DEST"
echo "It will start on the next login (or test now: env DISPLAY=:0 python3 \"$TARGET\")"
