#!/bin/bash

HOME_DIR=$HOME

# set profile name in ENV
#PROFILE_NAME=${PROFILE_NAME}

if [ $# -eq 0 ]; then
    echo "Usage: ./build.sh <profile_name> <platform>"
    exit 1
fi

PROFILE_NAME=$1

make clean

/usr/bin/make -j13 \
ARCH= CROSS_COMPILE=$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_t/host/bin/mipsel-linux- \
CFLAGS="-DPLATFORM_$2 -O0 -DALLOW_RTSP_SERVER_PORT_REUSE=1 -DNO_OPENSSL=1 -I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_t/host/mipsel-buildroot-linux-musl/sysroot/usr/include \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_t/host/mipsel-buildroot-linux-musl/sysroot/usr/include/freetype2 \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_t/host/mipsel-buildroot-linux-musl/sysroot/usr/include/liveMedia \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_t/host/mipsel-buildroot-linux-musl/sysroot/usr/include/groupsock \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_t/host/mipsel-buildroot-linux-musl/sysroot/usr/include/UsageEnvironment \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_t/host/mipsel-buildroot-linux-musl/sysroot/usr/include/BasicUsageEnvironment" \
LDFLAGS=" -L$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_t/host/mipsel-buildroot-linux-musl/sysroot/usr/lib \
-L$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_t/target/usr/lib" \
-C $PWD all

cp bin/prudynt /mnt/backup/software/development_root/nfs_root/
