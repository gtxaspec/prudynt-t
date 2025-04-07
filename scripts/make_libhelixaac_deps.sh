#!/bin/bash
# -----------------------------------------------------------------------------
# This script automates the process of setting up a cross-compilation
# environment for the Helix-AAC library using CMake. It prepares the build
# directory, sets the toolchain for cross-compilation, clones the Helix-AAC
# repository if not present, configures the build using CMake, compiles the
# library, and finally copies the built library and relevant headers to the
# appropriate locations in the repository.
# -----------------------------------------------------------------------------

set -e
set -o pipefail

# Variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../3rdparty"
LIBHELIX_REPO="https://github.com/earlephilhower/ESP8266Audio.git"
LIBHELIX_DIR="${BUILD_DIR}/libhelix-aac"
LIBHELIX_VER="master"
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

# Create libhelix-aac build directory
echo "Creating libhelix-aac build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Clone libhelix-aac if not already present
if [ ! -d "$LIBHELIX_DIR" ]; then
    echo "Cloning libhelix-aac..."
    git clone "$LIBHELIX_REPO" libhelix-aac
fi

cd "$LIBHELIX_DIR"

# Checkout desired version
if [[ -n "$LIBHELIX_VER" && "$LIBHELIX_VER" != "master" ]]; then
    git checkout $LIBHELIX_VER
else
    echo "Using libhelix-aac master branch"
fi

# Apply the patch
git apply ../../res/libhelix-aac.patch

# Create and navigate to build directory
mkdir -p build
cd build

# Configure the libhelix-aac build with CMake
echo "Configuring libhelix-aac library..."
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
    ../src/libhelix-aac/

echo "Building libhelix-aac library..."
make -j$(nproc)

if [ -f "${BUILD_DIR}/install/lib/libhelix-aac.so" ]; then
	$STRIP "${BUILD_DIR}/install/lib/libhelix-aac.so"
fi

# Install libhelix-aac library and headers
echo "Installing libhelix-aac library and headers..."
make install

echo "libhelix-aac build complete!"

