#!/bin/bash
# -----------------------------------------------------------------------------
# This script automates the process of setting up a cross-compilation
# environment for the libwebsockets library. It prepares the build
# directory, sets the toolchain for cross-compilation, clones the
# libwebsockets repository if not present, configures the build using CMake,
# compiles the library, and finally copies the built library and relevant
# headers to the appropriate locations in the repository.
# -----------------------------------------------------------------------------

set -e
set -o pipefail

# Variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../3rdparty"
LWS_REPO="https://github.com/warmcat/libwebsockets"
LWS_DIR="${BUILD_DIR}/libwebsockets"
LWS_VER="4415e84c095857629863804e941b9e1c2e9347ef"
MAKEFILE="$SCRIPT_DIR/../Makefile"

PRUDYNT_CROSS="${PRUDYNT_CROSS#ccache }"

CC="${PRUDYNT_CROSS}gcc"
CXX="${PRUDYNT_CROSS}g++"
STRIP="${PRUDYNT_CROSS}strip --strip-unneeded"

# Create libwebsockets build directory
echo "Creating libwebsockets build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Clone libwebsockets if not already present
if [ ! -d "$LWS_DIR" ]; then
    echo "Cloning libwebsockets..."
    git clone "$LWS_REPO"
fi

cd "$LWS_DIR"

# Checkout desired version
if [[ -n "$LWS_VER" ]]; then
    git checkout $LWS_VER
else
    echo "Pulling libwebsockets master"
fi

# Create and navigate to cmake build dir
mkdir -p build
cd build

# Configure and build libwebsockets library

echo "Configuring libwebsockets library..."
cmake \
-DCMAKE_SYSTEM_NAME=Linux \
-DCMAKE_SYSTEM_PROCESSOR=mipsle \
-DCMAKE_C_COMPILER_LAUNCHER=$(which ccache) \
-DCMAKE_CXX_COMPILER_LAUNCHER=$(which ccache) \
-DCMAKE_C_COMPILER=${CC} \
-DCMAKE_CXX_COMPILER=${CXX} \
-DCMAKE_BUILD_TYPE=RELEASE \
-DCMAKE_C_FLAGS="${CMAKE_C_FLAGS} -Os" \
-DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -Os" \
-DDISABLE_WERROR=ON \
-DLWS_WITH_DIR=OFF \
-DLWS_WITH_LEJP_CONF=OFF \
-DLWS_WITHOUT_EXTENSIONS=1 \
-DLWS_WITH_NETLINK=OFF \
-DLWS_WITH_SSL=OFF \
-DLWS_HAVE_LIBCAP=OFF \
-DLWS_CTEST_INTERNET_AVAILABLE=OFF \
-DLWS_WITH_MINIMAL_EXAMPLES=OFF \
-DLWS_IPV6=ON \
-DLWS_ROLE_RAW_FILE=OFF \
-DLWS_UNIX_SOCK=OFF \
-DLWS_WITH_HTTP_BASIC_AUTH=OFF \
-DLWS_WITH_HTTP_UNCOMMON_HEADERS=OFF \
-DLWS_WITH_SYS_STATE=OFF \
-DLWS_WITH_SYS_SMD=OFF \
-DLWS_WITH_UPNG=OFF \
-DLWS_WITH_GZINFLATE=OFF \
-DLWS_WITH_JPEG=OFF \
-DLWS_WITH_DLO=OFF \
-DLWS_WITH_SECURE_STREAMS=OFF \
-DLWS_SSL_CLIENT_USE_OS_CA_CERTS=OFF \
-DLWS_WITH_TLS_SESSIONS=OFF \
-DLWS_WITH_EVLIB_PLUGINS=OFF \
-DLWS_WITH_LEJP=ON \
-DLWS_WITH_CBOR_FLOAT=OFF \
-DLWS_WITH_LHP=OFF \
-DLWS_WITH_JSONRPC=OFF \
-DLWS_WITH_LWSAC=ON \
-DLWS_WITH_CUSTOM_HEADERS=OFF \
-DLWS_CLIENT_HTTP_PROXYING=OFF \
-DLWS_WITH_FILE_OPS=ON \
-DLWS_WITH_CONMON=OFF \
-DLWS_WITH_CACHE_NSCOOKIEJAR=OFF \
-DLWS_WITHOUT_TESTAPPS=ON \
..

echo "Building libwebsockets library..."
make -j$(nproc)

# Copy libwebsockets library and headers
echo "Copying libwebsockets library and headers..."
$STRIP ./lib/libwebsockets.so.19

mkdir -p ../../install/
mkdir -p ../../install/lib/
if [[ "$1" == "-static" ]]; then
	cp ./lib/libwebsockets.a ../../install/lib/
else
	cp ./lib/libwebsockets.so.19 ../../install/lib/libwebsockets.so
fi
#cp -R ../include/libwebsockets ../../../include/
cp -R include/* ../../install/include/

echo "libwebsockets build complete!"
