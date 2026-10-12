#!/usr/bin/env bash
# Build Open3DStream on Linux (the Windows equivalent is build.bat).
#
#   ./build.sh [Debug|Release|RelWithDebInfo|MinSizeRel]   (default: RelWithDebInfo)
#
# 1. Builds the third-party libraries (thirdparty/: flatbuffers, nng, crccpp)
#    and installs them into ./usr. Skipped if ./usr already exists; remove it
#    to force a rebuild.
# 2. Configures and builds the project in ./build, installing into ./usr.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG="${1:-RelWithDebInfo}"
JOBS="$(nproc 2>/dev/null || echo 4)"
PREFIX="$ROOT/usr"

case "$CONFIG" in
    Debug|Release|RelWithDebInfo|MinSizeRel) ;;
    *) echo "Unknown build type: $CONFIG" >&2; exit 1 ;;
esac

command -v cmake >/dev/null || { echo "ERROR: cmake not found" >&2; exit 1; }

# Make sure the third-party submodules are checked out
if [ ! -f "$ROOT/thirdparty/nng/CMakeLists.txt" ] || [ ! -f "$ROOT/thirdparty/flatbuffers/CMakeLists.txt" ] || [ ! -f "$ROOT/thirdparty/crccpp/CMakeLists.txt" ]; then
    echo "==> Initialising third-party submodules"
    git -C "$ROOT" submodule update --init thirdparty/flatbuffers thirdparty/nng thirdparty/crccpp
fi

if [ ! -d "$PREFIX" ]; then
    # mbedtls first: nng's TLS transport (PSK) is built against it
    echo "==> Building mbedtls ($CONFIG)"
    cmake -S "$ROOT/thirdparty/mbedtls" -B "$ROOT/thirdparty/build/mbedtls" \
        -DCMAKE_BUILD_TYPE="$CONFIG" \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DENABLE_PROGRAMS=OFF -DENABLE_TESTING=OFF -DMBEDTLS_FATAL_WARNINGS=OFF
    cmake --build "$ROOT/thirdparty/build/mbedtls" --target install --parallel "$JOBS"

    echo "==> Building third-party libraries ($CONFIG)"
    cmake -S "$ROOT/thirdparty" -B "$ROOT/thirdparty/build" \
        -DCMAKE_BUILD_TYPE="$CONFIG" \
        -DCMAKE_PREFIX_PATH="$PREFIX" \
        -DCMAKE_INSTALL_PREFIX="$PREFIX"
    cmake --build "$ROOT/thirdparty/build" --target install --parallel "$JOBS"
else
    echo "==> Using existing third-party install in $PREFIX"
fi

echo "==> Configuring Open3DStream ($CONFIG)"
cmake -S "$ROOT" -B "$ROOT/build" \
    -DCMAKE_BUILD_TYPE="$CONFIG" \
    -DCMAKE_PREFIX_PATH="$PREFIX" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX"

echo "==> Building and installing Open3DStream"
cmake --build "$ROOT/build" --target install --parallel "$JOBS"

echo "==> Done. Output installed to $PREFIX"
