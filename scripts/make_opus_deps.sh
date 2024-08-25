#!/bin/bash
# -----------------------------------------------------------------------------
# This script automates the process of setting up a cross-compilation
# environment for the Opus library using CMake. It prepares the build
# directory, sets the toolchain for cross-compilation, clones the Opus
# repository if not present, configures the build using CMake, compiles the
# library, and finally copies the built library and relevant headers to the
# appropriate locations in the repository.
# -----------------------------------------------------------------------------

set -e
set -o pipefail

# Variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../3rdparty"
OPUS_REPO="https://github.com/xiph/opus"
OPUS_DIR="${BUILD_DIR}/opus"
OPUS_VER="82ac57d9f1aaf575800cf17373348e45b7ce6c0d"
MAKEFILE="$SCRIPT_DIR/../Makefile"

PRUDYNT_CROSS="${PRUDYNT_CROSS#ccache }"

CC="${PRUDYNT_CROSS}gcc"
CXX="${PRUDYNT_CROSS}g++"
STRIP="${PRUDYNT_CROSS}strip --strip-unneeded"

# Determine if building static or shared library
BUILD_SHARED_LIBS=ON
if [[ "$1" == "-static" ]]; then
    BUILD_SHARED_LIBS=OFF
fi

# Create Opus build directory
echo "Creating Opus build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Clone Opus if not already present
if [ ! -d "$OPUS_DIR" ]; then
    echo "Cloning Opus..."
    git clone "$OPUS_REPO"
fi

cd "$OPUS_DIR"

# Checkout desired version
if [[ -n "$OPUS_VER" ]]; then
    git checkout $OPUS_VER
else
    echo "Pulling Opus master"
fi

# Create and navigate to build directory
mkdir -p build
cd build

# Configure the Opus build with CMake
echo "Configuring Opus library..."
cmake \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=mipsle \
    -DCMAKE_C_COMPILER=${CC} \
    -DCMAKE_CXX_COMPILER=${CXX} \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-Os" \
    -DCMAKE_CXX_FLAGS="-Os" \
    -DCMAKE_INSTALL_PREFIX="${BUILD_DIR}/install" \
    -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} \
    ..

echo "Building Opus library..."
make -j$(nproc)

if [ -f "${BUILD_DIR}/install/lib/libopus.so" ]; then
	$STRIP "${BUILD_DIR}/install/lib/libopus.so"
fi

# Install Opus library and headers
echo "Installing Opus library and headers..."
make install

echo "Opus build complete!"
