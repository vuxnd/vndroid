#!/bin/bash
# To customize the version name:
#     VERSION=myversion ./release.sh
set -e

cd "$(dirname "${BASH_SOURCE[0]}")"
rm -rf output

./test_server.sh
./test_client.sh

./build_server.sh
./build_windows.sh 32
./build_windows.sh 64

./package_server.sh
./package_client.sh win32 zip
./package_client.sh win64 zip

# SHA256SUMS.txt comes from the release workflow, which builds all five
# targets (winarm64 and the macOS dmgs cannot be built here)

echo "Release generated in $PWD/output"
