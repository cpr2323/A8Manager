#!/bin/bash
# Packages a Linux build of A8Manager as A8Manager-<version>-linux-x86_64.tar.gz
#
#   package.sh [build output folder] [destination folder]
#
# The build output folder is the one holding the A8Manager binary and its Licenses folder,
# and defaults to build/A8Manager_artefacts/Release. The destination defaults to build/dist.
#
# Build on the oldest distro the package should run on: the binary will not start on a
# system with an older glibc than the one it was built against.
set -e

here="$(cd "$(dirname "$0")" && pwd)"
repo="$(cd "$here/../.." && pwd)"
build_dir="${1:-$repo/build/A8Manager_artefacts/Release}"
dest_dir="${2:-$repo/build/dist}"

version="$(sed -n 's/^project(A8Manager VERSION \([0-9.]*\)).*/\1/p' "$repo/CMakeLists.txt" | tr -d '\r')"
icon_source="$repo/Source/GUI/Assimil8or/Data/377906243_984640136172516_2914152204379747274_n.png"

[ -n "$version" ] || { echo "Could not read the version from $repo/CMakeLists.txt"; exit 1; }
[ -x "$build_dir/A8Manager" ] || { echo "No A8Manager binary in $build_dir"; exit 1; }
[ -d "$build_dir/Licenses" ] || { echo "No Licenses folder in $build_dir"; exit 1; }
command -v convert > /dev/null || { echo "ImageMagick is needed to size the icon: sudo apt install imagemagick"; exit 1; }

name="A8Manager-$version-linux-x86_64"
staging="$(mktemp -d)"
trap 'rm -rf "$staging"' EXIT
package="$staging/$name"
mkdir -p "$package"

cp "$build_dir/A8Manager" "$package/"
strip --strip-unneeded "$package/A8Manager"
cp -r "$build_dir/Licenses" "$package/"
cp "$here/a8manager.desktop" "$here/install.sh" "$here/README.txt" "$package/"
# Set the modes outright, since files copied from a Windows drive under WSL all look executable
chmod 755 "$package/A8Manager" "$package/install.sh"
chmod 644 "$package/a8manager.desktop" "$package/README.txt"

# The source icon is 514x514; desktops only look for the standard sizes
convert "$icon_source" -strip -resize 256x256 "$package/a8manager.png"

glibc="$(objdump -T "$package/A8Manager" | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1)"

mkdir -p "$dest_dir"
tar -C "$staging" --owner=0 --group=0 -czf "$dest_dir/$name.tar.gz" "$name"

echo "Created $dest_dir/$name.tar.gz"
echo "Needs $glibc or newer"
