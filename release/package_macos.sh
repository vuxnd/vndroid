#!/bin/bash
set -ex
cd "$(dirname ${BASH_SOURCE[0]})"
. build_common
cd .. # root project dir

if [[ $# != 1 ]]
then
    echo "Syntax: $0 <arch>" >&2
    exit 1
fi

ARCH="$1"
BUILD_DIR="$WORK_DIR/build-macos-$ARCH"
DMG_DIR="$BUILD_DIR/release-dmg"
APP="$DMG_DIR/VNdroid.app"
ICONSET="$BUILD_DIR/vndroid.iconset"
DMG="$OUTPUT_DIR/vndroid-macos-$ARCH-$VERSION.dmg"

rm -rf "$DMG_DIR" "$ICONSET"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources" "$ICONSET"

# The client is built with -Dportable=true: it looks for adb, vndroid-server
# and the icon next to its own executable
cp "$BUILD_DIR/dist/vndroid" "$BUILD_DIR/dist/adb" \
    "$BUILD_DIR/dist/vndroid.png" "$BUILD_DIR/dist/disconnected.png" \
    "$WORK_DIR/build-server/server/vndroid-server" "$APP/Contents/MacOS/"
# Not named VNdroid: it would overwrite vndroid on a case-insensitive volume
cp app/data/macos/launcher "$APP/Contents/MacOS/VNdroid-launcher"
chmod +x "$APP/Contents/MacOS/VNdroid-launcher"
cp app/data/macos/update "$APP/Contents/MacOS/vndroid-update"
chmod +x "$APP/Contents/MacOS/vndroid-update"

sed "s/@VERSION@/${VERSION#v}/g" app/data/macos/Info.plist > "$APP/Contents/Info.plist"

for size in 16 32 128 256 512
do
    sips -z $size $size app/data/vndroid.png --out "$ICONSET/icon_${size}x${size}.png"
    sips -z $((size * 2)) $((size * 2)) app/data/vndroid.png \
        --out "$ICONSET/icon_${size}x${size}@2x.png"
done
iconutil -c icns "$ICONSET" -o "$APP/Contents/Resources/vndroid.icns"

# Ad-hoc signature, required on Apple silicon. adb keeps its own signature, and
# the bundle is not sealed because its main executable is a script.
codesign --force --sign - "$APP/Contents/MacOS/vndroid"

cp LICENSE "$DMG_DIR/"
ln -s /Applications "$DMG_DIR/Applications"

mkdir -p "$OUTPUT_DIR"
rm -f "$DMG"
hdiutil create -volname VNdroid -srcfolder "$DMG_DIR" -format UDZO "$DMG"

echo "Generated '$DMG'"
