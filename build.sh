#!/bin/bash
set -e
PRUDYNT_CROSS='ccache mipsel-linux-'
PRUDYNT_CROSS2=mipsel-linux-

TOP=$(pwd)
echo "TOP = $TOP"

echo "Build prudynt"
rm -rf build
mkdir build
cd build
cmake -DPRUDYNT_CROSS="${PRUDYNT_CROSS2}" ..
make
cd ..

patchelf --set-rpath . build/prudynt
cp build/prudynt /mnt/backup/software/development_root/nfs_root/PRU/
cp 3rdparty/install/lib/* /mnt/backup/software/development_root/nfs_root/PRU/
