#!/bin/bash

HOME_DIR=$HOME

# set profile name in ENV
PROFILE_NAME=${PROFILE_NAME}

make clean

/usr/bin/make -j13 \
ARCH= CROSS_COMPILE=$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_v3/host/bin/mipsel-linux- \
CFLAGS="-DNO_OPENSSL=1 -Og -g -I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_v3/host/mipsel-buildroot-linux-musl/sysroot/usr/include \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_v3/host/mipsel-buildroot-linux-musl/sysroot/usr/include/freetype2 \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_v3/host/mipsel-buildroot-linux-musl/sysroot/usr/include/liveMedia \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_v3/host/mipsel-buildroot-linux-musl/sysroot/usr/include/groupsock \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_v3/host/mipsel-buildroot-linux-musl/sysroot/usr/include/UsageEnvironment \
-I$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_v3/host/mipsel-buildroot-linux-musl/sysroot/usr/include/BasicUsageEnvironment" \
LDFLAGS=" -L$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_v3/host/mipsel-buildroot-linux-musl/sysroot/usr/lib \
-L$HOME_DIR/output/$PROFILE_NAME/per-package/prudynt_v3/target/usr/lib" \
-C $PWD all

cp bin/prudynt /mnt/backup/software/development_root/nfs_root/
