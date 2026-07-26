#!/bin/sh
set -eu

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "Aufruf: $0 VERSION SEED_APPIMAGE [OUTPUT_APPIMAGE]" >&2
    exit 2
fi

version=$1
seed_appimage=$(readlink -f "$2")
output_appimage=${3:-"dist/RickSheets-${version}-x86_64.AppImage"}
output_appimage=$(readlink -m "$output_appimage")
project_root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
binary="$project_root/build-release/ricksheets"
appimagetool=${APPIMAGETOOL:-}
runtime_file=${APPIMAGE_RUNTIME_FILE:-}
pdftoppm=${PDFTOPPM:-$(command -v pdftoppm || true)}

if [ ! -x "$seed_appimage" ]; then
    echo "Seed-AppImage fehlt oder ist nicht ausführbar: $seed_appimage" >&2
    exit 1
fi
if [ ! -x "$binary" ]; then
    echo "Release-Binary fehlt: $binary" >&2
    echo "Zuerst cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release und den Build ausführen." >&2
    exit 1
fi
if ! strings "$binary" | grep -Fxq "$version"; then
    echo "Release-Binary enthält nicht die erwartete Version $version." >&2
    exit 1
fi
if ! ldd "$binary" | grep -q 'libQt6Core'; then
    echo "Release-Binary ist kein geprüfter Qt-6-Build." >&2
    exit 1
fi
if [ -z "$appimagetool" ] || [ ! -x "$appimagetool" ]; then
    echo "APPIMAGETOOL muss auf eine ausführbare lokale appimagetool-Datei zeigen." >&2
    exit 1
fi

work_dir=$(mktemp -d "${TMPDIR:-/tmp}/ricksheets-appimage-${version}.XXXXXX")
cleanup()
{
    rm -rf -- "$work_dir"
}
trap cleanup EXIT HUP INT TERM

(
    cd "$work_dir"
    "$seed_appimage" --appimage-extract >/dev/null
)
app_dir="$work_dir/squashfs-root"
cmake --install "$project_root/build-release" --prefix "$app_dir/usr"
if [ -n "$pdftoppm" ] && [ -x "$pdftoppm" ]; then
    install -m 0755 "$pdftoppm" "$app_dir/usr/bin/pdftoppm"
fi

# Replace the historical pre-GitHub desktop integration inherited from the
# seed image. The files below only exist in the fresh temporary AppDir.
rm -f -- \
    "$app_dir/de.rickrich.RickSheets.desktop" \
    "$app_dir/de.rickrich.RickSheets.png" \
    "$app_dir/usr/share/applications/de.rickrich.RickSheets.desktop"
cp "$app_dir/usr/share/applications/io.github.erixx_hub.RickSheets.desktop" \
   "$app_dir/io.github.erixx_hub.RickSheets.desktop"
cp "$app_dir/usr/share/icons/hicolor/256x256/apps/io.github.erixx_hub.RickSheets.png" \
   "$app_dir/io.github.erixx_hub.RickSheets.png"

if ! LD_LIBRARY_PATH="$app_dir/usr/lib" ldd "$app_dir/usr/bin/ricksheets" |
     grep -q "$app_dir/usr/lib/libQt6Core"; then
    echo "Das Seed-AppImage stellt die benötigten Qt-6-Bibliotheken nicht bereit." >&2
    exit 1
fi
if [ ! -x "$app_dir/usr/bin/pdftoppm" ]; then
    echo "pdftoppm fehlt; PDF-Seitenvorschau könnte nicht paketiert werden." >&2
    exit 1
fi
if LD_LIBRARY_PATH="$app_dir/usr/lib" ldd "$app_dir/usr/bin/pdftoppm" |
   grep -q 'not found'; then
    echo "Das AppDir enthält nicht alle Laufzeitbibliotheken für pdftoppm." >&2
    exit 1
fi

if [ -z "$runtime_file" ]; then
    runtime_file="$work_dir/runtime-x86_64"
    runtime_size=$("$seed_appimage" --appimage-offset)
    dd if="$seed_appimage" of="$runtime_file" bs=1 count="$runtime_size" status=none
fi
if [ ! -f "$runtime_file" ]; then
    echo "AppImage-Runtime fehlt: $runtime_file" >&2
    exit 1
fi

mkdir -p "$(dirname -- "$output_appimage")"
"$appimagetool" --runtime-file "$runtime_file" "$app_dir" "$output_appimage"
sha256sum "$binary" "$output_appimage"
