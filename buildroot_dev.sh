#!/bin/bash

HOME_DIR=$HOME
DEST_DIR=${DEST_DIR}

# set profile name in ENV
#PROFILE_NAME=${PROFILE_NAME}

if [ $# -eq 0 ]; then
    echo "Usage: ./build.sh <profile_name> <platform>"
    exit 1
fi

PROFILE_NAME=$1

make clean

/usr/bin/make -j13 \
ARCH= CROSS_COMPILE=$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt-t/host/bin/mipsel-linux- \
CFLAGS="-DPLATFORM_$2 -Os -DALLOW_RTSP_SERVER_PORT_REUSE=1 -DNO_OPENSSL=1 -I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt-t/host/mipsel-buildroot-linux-musl/sysroot/usr/include \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt-t/host/mipsel-buildroot-linux-musl/sysroot/usr/include/freetype2 \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt-t/host/mipsel-buildroot-linux-musl/sysroot/usr/include/liveMedia \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt-t/host/mipsel-buildroot-linux-musl/sysroot/usr/include/groupsock \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt-t/host/mipsel-buildroot-linux-musl/sysroot/usr/include/UsageEnvironment \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt-t/host/mipsel-buildroot-linux-musl/sysroot/usr/include/BasicUsageEnvironment" \
LDFLAGS=" -L$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt-t/host/mipsel-buildroot-linux-musl/sysroot/usr/lib \
-L$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt-t/target/usr/lib" \
-C $PWD all

if [[ $DEST_DIR == "" ]]; then
	echo "DEST_DIR not set"
else
	cp bin/prudynt $DEST_DIR
fi
