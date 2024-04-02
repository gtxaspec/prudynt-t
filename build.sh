#!/bin/bash
set -e

PRUDYNT_CROSS="ccache mipsel-linux-"

TOP=$(pwd)
echo "TOP = $TOP"


if [ $# -eq 0 ]; then
    echo "Usage: ./build.sh <platform>"
    echo "Platform: T20/T21/T23/T30/T31"
    exit 1
fi

rm -rf 3rdparty
mkdir -p 3rdparty/install

echo "Build freetype2"
cd 3rdparty
rm -rf freetype
if [[ ! -f freetype-2.13.2.tar.xz ]]; then
    wget 'https://download-mirror.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.xz'
fi
tar xvf freetype-2.13.2.tar.xz
mv freetype-2.13.2 freetype
cd freetype
CC="${PRUDYNT_CROSS}gcc" ./configure --host mipsel-linux-gnu --prefix="$TOP/3rdparty/install/" --without-harfbuzz --disable-largefile --disable-mmap --without-png --without-brotli --without-zlib
make -j$(nproc)
make install
cd ../../

echo "Build libconfig"
cd 3rdparty
rm -rf libconfig
if [[ ! -f libconfig-1.7.3.tar.gz ]]; then
    wget 'https://hyperrealm.github.io/libconfig/dist/libconfig-1.7.3.tar.gz';
fi
tar xvf libconfig-1.7.3.tar.gz
mv libconfig-1.7.3 libconfig
cd libconfig
CC="${PRUDYNT_CROSS}gcc" CXX="${PRUDYNT_CROSS}g++" ./configure --host mipsel-linux-gnu --prefix="$TOP/3rdparty/install"
make -j$(nproc)
make install
cd ../../

echo "Build live555"
cd 3rdparty
rm -rf live555
if [[ ! -f live555-latest.tar.gz ]]; then
	wget 'http://www.live555.com/liveMedia/public/live555-latest.tar.gz';
fi
tar xvf live555-latest.tar.gz
cd live
if [[ -f Makefile ]]; then
    make distclean
fi
cp ../../res/config.wyze .
./genMakefiles wyze
PRUDYNT_ROOT="${TOP}" PRUDYNT_CROSS="${PRUDYNT_CROSS}" make -j$(nproc)
PRUDYNT_ROOT="${TOP}" PRUDYNT_CROSS="${PRUDYNT_CROSS}" make install
cd ../../

echo "import libimp"
cd 3rdparty
rm -rf ingenic-lib
if [[ ! -d ingenic-lib ]]; then
git clone https://github.com/gtxaspec/ingenic-lib
cp ingenic-lib/$1/lib/1.1.6/uclibc/5.4.0/* $TOP/3rdparty/install/lib
fi
cd ..

echo "Build prudynt"

make clean
/usr/bin/make -j13 \
ARCH= CROSS_COMPILE=mipsel-linux- \
CFLAGS="-DPLATFORM_$1 -Os -DALLOW_RTSP_SERVER_PORT_REUSE=1 -DNO_OPENSSL=1 -I./3rdparty/install/include \
-I./3rdparty/install/include/freetype2 \
-I./3rdparty/install/include/liveMedia \
-I./3rdparty/install/include/groupsock \
-I./3rdparty/install/include/UsageEnvironment \
-I./3rdparty/install/include/BasicUsageEnvironment" \
LDFLAGS=" -L./3rdparty/install/lib" \
-C $PWD all
