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
LWS_VER="b0a749c8e7a8294b68581ce4feac0e55045eb00"
MAKEFILE="$SCRIPT_DIR/../Makefile"

echo $MAKEFILE
# Set compiler prefix here
if [ -n "$BR2_CONFIG" ]; then
    echo "Running within Buildroot build process."
else

if grep -q "CONFIG_UCLIBC_BUILD=y" "$MAKEFILE"; then
	echo "Build type: uClibc"
	CROSS_COMPILE="mips-linux-uclibc-gnu-"
elif grep -q "CONFIG_MUSL_BUILD=y" "$MAKEFILE"; then
	echo "Build type: musl"
	CROSS_COMPILE="mipsel-linux-"
elif grep -q "CONFIG_GCC_BUILD=y" "$MAKEFILE"; then
	echo "Build type: GCC"
	CROSS_COMPILE="mipsel-linux-gnu-"
fi

fi

CC="${CROSS_COMPILE}gcc"
CXX="${CROSS_COMPILE}g++"
STRIP="${CROSS_COMPILE}strip --strip-unneeded"

# Ensure CC and CXX are set
if [ -z "$CC" ] || [ -z "$CXX" ]; then
    echo "Error: CC and CXX environment variables must be set."
    exit 1
fi

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

if grep -q "CONFIG_STATIC_BUILD=n" "$MAKEFILE"; then
echo "Found CONFIG_STATIC_BUILD=n"
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

elif grep -q "CONFIG_STATIC_BUILD=y" "$MAKEFILE"; then
echo "Found CONFIG_STATIC_BUILD=y"
echo "Configuring libwebsockets library..."
cmake \
-DCMAKE_SYSTEM_NAME=Linux \
-DCMAKE_SYSTEM_PROCESSOR=mipsle \
-DCMAKE_C_COMPILER_LAUNCHER=$(which ccache) \
-DCMAKE_CXX_COMPILER_LAUNCHER=$(which ccache) \
-DCMAKE_C_COMPILER=${CC} \
-DCMAKE_CXX_COMPILER=${CXX} \
-DCMAKE_BUILD_TYPE=RELEASE \
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
-DLWS_WITH_LEJP=OFF \
-DLWS_WITH_CBOR_FLOAT=OFF \
-DLWS_WITH_LHP=OFF \
-DLWS_WITH_JSONRPC=OFF \
-DLWS_WITH_LWSAC=OFF \
-DLWS_WITH_CUSTOM_HEADERS=OFF \
-DLWS_CLIENT_HTTP_PROXYING=OFF \
-DLWS_WITH_FILE_OPS=OFF \
-DLWS_WITH_CONMON=OFF \
-DLWS_WITH_CACHE_NSCOOKIEJAR=OFF \
-DLWS_WITHOUT_TESTAPPS=ON \
..
else
echo "CONFIG_STATIC_BUILD setting not found or is set to an unexpected value."
fi

echo "Building libwebsockets library..."
make

# Copy libwebsockets library and headers
echo "Copying libwebsockets library and headers..."
$STRIP ./lib/libwebsockets.so.19

mkdir -p ../../install/
mkdir -p ../../install/lib/
cp ./lib/libwebsockets.a ../../install/lib/
cp ./lib/libwebsockets.so.19 ../../install/lib/libwebsockets.so
#cp -R ../include/libwebsockets ../../../include/
cp -R include/* ../../install/include/

echo "libwebsockets build complete!"
