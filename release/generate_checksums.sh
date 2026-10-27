#!/bin/bash
set -ex -o pipefail
cd "$(dirname ${BASH_SOURCE[0]})"
. build_common

# The app downloads vndroid-<target>-<tag>.<ext> from the release of that tag
if [[ ! "$VERSION" =~ ^v[0-9]+(\.[0-9]+)*$ ]]
then
    echo "ERROR: VERSION '$VERSION' is not a release tag (vX.Y.Z), but the app downloads vndroid-<target>-<tag>.<ext>" >&2
    exit 1
fi

cd "$OUTPUT_DIR"

# The server + one asset per SC_UPDATE_TARGET in app/src/update.h
ASSETS=(
    "vndroid-server-$VERSION"
    "vndroid-win32-$VERSION.zip"
    "vndroid-win64-$VERSION.zip"
    "vndroid-winarm64-$VERSION.zip"
    "vndroid-macos-aarch64-$VERSION.dmg"
    "vndroid-macos-x86_64-$VERSION.dmg"
)

for asset in "${ASSETS[@]}"
do
    if [[ ! -f "$asset" ]]
    then
        echo "ERROR: missing $asset" >&2
        exit 1
    fi
done

sha256sum "${ASSETS[@]}" | tee SHA256SUMS.txt
echo "Release checksums generated in $PWD/SHA256SUMS.txt"
