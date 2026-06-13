#!/usr/bin/env bash
set -euxo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VCPKG_ROOT="$HOME/vcpkg"
VCPKG_PREFIX="$ROOT/vcpkg_installed/x64-linux"

CGAL_DIR="$VCPKG_PREFIX/share/cgal"
TL_DIR="$VCPKG_PREFIX/share/cmake/tl-function-ref"
TOOLCHAIN="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

test -f "$TOOLCHAIN"
test -f "$CGAL_DIR/CGALConfig.cmake"
test -f "$TL_DIR/tl-function-ref-config.cmake"

rm -rf "$ROOT/build"

cmake \
  -S "$ROOT" \
  -B "$ROOT/build" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DENABLE_TESTING=ON \
  -DCMAKE_TOOLCHAIN_FILE:FILEPATH="$TOOLCHAIN" \
  -DVCPKG_TARGET_TRIPLET:STRING=x64-linux \
  -DVCPKG_MANIFEST_MODE:BOOL=ON \
  -D_VCPKG_INSTALLED_DIR:PATH="$ROOT/vcpkg_installed" \
  -DCMAKE_PREFIX_PATH:PATH="$VCPKG_PREFIX" \
  -DCGAL_DIR:PATH="$CGAL_DIR" \
  -D"tl-function-ref_DIR:PATH=$TL_DIR"

cmake --build "$ROOT/build" --parallel "$(nproc)"
ctest --test-dir "$ROOT/build" --output-on-failure -j2
