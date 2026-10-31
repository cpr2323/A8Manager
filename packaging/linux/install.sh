#!/bin/bash
# Installs A8Manager for the current user - no root needed - or removes it again
# with --uninstall. Everything goes under ~/.local, and settings are left alone.
set -e

here="$(cd "$(dirname "$0")" && pwd)"
data="${XDG_DATA_HOME:-$HOME/.local/share}"
app_dir="$data/A8Manager"
bin_link="$HOME/.local/bin/A8Manager"
desktop_file="$data/applications/a8manager.desktop"
icon_file="$data/icons/hicolor/256x256/apps/a8manager.png"

# Desktops pick up new launchers on their own, but this makes it immediate where it exists.
# The icon cache is deliberately not rebuilt: a cache in ~/.local goes stale and hides the
# icons of anything installed there later, while without one the folder is just scanned.
refresh ()
{
    if command -v update-desktop-database > /dev/null; then update-desktop-database "$data/applications" || true; fi
}

if [ "$1" = "--uninstall" ]; then
    rm -rf "$app_dir"
    rm -f "$bin_link" "$desktop_file" "$icon_file"
    refresh
    echo "A8Manager has been removed."
    exit 0
fi

mkdir -p "$app_dir" "$(dirname "$bin_link")" "$(dirname "$desktop_file")" "$(dirname "$icon_file")"

rm -rf "$app_dir/Licenses"
cp "$here/A8Manager" "$app_dir/A8Manager"
cp -r "$here/Licenses" "$app_dir/Licenses"
chmod +x "$app_dir/A8Manager"
ln -sf "$app_dir/A8Manager" "$bin_link"

# The launcher points at the binary itself, since ~/.local/bin is not on every desktop's PATH
sed "s|^Exec=.*|Exec=\"$app_dir/A8Manager\"|" "$here/a8manager.desktop" > "$desktop_file"
cp "$here/a8manager.png" "$icon_file"

refresh

echo "A8Manager has been installed. It is in your applications menu, or run: $bin_link"
