#!/bin/bash
set -ex
cd "$(dirname ${BASH_SOURCE[0]})"
. build_common

cd "$OUTPUT_DIR"
sha256sum "vndroid-server-$VERSION" \
    "vndroid-linux-x86_64-$VERSION.tar.gz" \
    "vndroid-win32-$VERSION.zip" \
    "vndroid-win64-$VERSION.zip" \
    "vndroid-winarm64-$VERSION.zip" \
    "vndroid-macos-aarch64-$VERSION.tar.gz" \
    "vndroid-macos-x86_64-$VERSION.tar.gz" \
        | tee SHA256SUMS.txt
echo "Release checksums generated in $PWD/SHA256SUMS.txt"
