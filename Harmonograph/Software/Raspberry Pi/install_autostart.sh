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

# Resolve the Python interpreter:
#   1) explicit:    VENV=/path/to/venv ./install_autostart.sh
#   2) activated:   run the installer from inside the venv
#   3) local:       a 'venv/' directory next to this script
#   4) fallback:    system python3
if [ -n "$VENV" ] && [ -x "$VENV/bin/python3" ]; then
    PY="$VENV/bin/python3"
elif [ -n "$VIRTUAL_ENV" ] && [ -x "$VIRTUAL_ENV/bin/python3" ]; then
    PY="$VIRTUAL_ENV/bin/python3"
elif [ -x "$DIR/venv/bin/python3" ]; then
    PY="$DIR/venv/bin/python3"
else
    PY="python3"
fi

# sleep 3 lets the desktop settle before the camera window grabs the screen
cat > "$DEST" <<EOF
[Desktop Entry]
Type=Application
Name=Harmonograph Camera
Comment=Harmonograph webcam display ($SCRIPT_NAME)
Exec=sh -c 'sleep 3; exec env DISPLAY=:0 "$PY" "$TARGET"'
Terminal=false
X-GNOME-Autostart-enabled=true
EOF
chmod +x "$DEST"

echo "Installed: $DEST"
echo "Interpreter: $PY"
echo "It will start on the next login (or test now: env DISPLAY=:0 \"$PY\" \"$TARGET\")"

# Also autostart unclutter (cursor hider) if it is installed.
if command -v unclutter >/dev/null 2>&1; then
    UC="$HOME/.config/autostart/unclutter.desktop"
    cat > "$UC" <<EOF
[Desktop Entry]
Type=Application
Name=Unclutter
Comment=Hide the mouse cursor when idle
Exec=sh -c 'sleep 4; exec env DISPLAY=:0 unclutter -idle 2'
Terminal=false
X-GNOME-Autostart-enabled=true
EOF
    chmod +x "$UC"
    echo "Installed: $UC (cursor hides after ~2s idle, reappears on movement)"
else
    echo "NOTE: unclutter not found - cursor stays visible."
    echo "      Install it with: sudo apt install unclutter, then re-run this script."
fi
