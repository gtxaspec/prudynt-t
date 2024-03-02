#!/bin/bash
set -e
PRUDYNT_CROSS='ccache mipsel-linux-'
PRUDYNT_CROSS2=mipsel-linux-

TOP=$(pwd)
echo "TOP = $TOP"

rm -rf 3rdparty/install
mkdir 3rdparty/install

echo "Build libconfig"
cd 3rdparty
rm -rf libconfig
if [[ ! -f libconfig-1.7.3.tar.gz ]]; then
    wget 'https://hyperrealm.github.io/libconfig/dist/libconfig-1.7.3.tar.gz';
fi
tar xvf libconfig-1.7.3.tar.gz
mv libconfig-1.7.3 libconfig
cd libconfig
CC="${PRUDYNT_CROSS}gcc" CXX="${PRUDYNT_CROSS}g++" ./configure --host mipsel-linux-gnu --prefix="$TOP/3rdparty/install" --disable-static
make -j$(nproc)
make install
cd ../../

echo "Build live555"
cd 3rdparty/live
#make distclean
./genMakefiles wyze
PRUDYNT_ROOT="${TOP}" PRUDYNT_CROSS="${PRUDYNT_CROSS}" make -j$(nproc)
PRUDYNT_ROOT="${TOP}" PRUDYNT_CROSS="${PRUDYNT_CROSS}" make install
cd ../../

echo "Build prudynt"
rm -rf build
mkdir build
cd build
cmake -DPRUDYNT_CROSS="${PRUDYNT_CROSS2}" ..
make
cd ..

patchelf --set-rpath . build/prudynt
cp 3rdparty/install/lib/* /mnt/backup/software/development_root/nfs_root/PRU/
sleep 1
sync
cp build/prudynt /mnt/backup/software/development_root/nfs_root/PRU/
