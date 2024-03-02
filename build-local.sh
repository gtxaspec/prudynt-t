#!/bin/bash
set -e

TOP=$(realpath ../../)
echo "TOP = $TOP"

rm -f prudynt
rm -rf CMakeCache.txt
rm -rf CMakeFiles/
rm -f Makefile
rm -f cmake_install.cmake

echo "Build prudynt from buildroot"
[ -d build ] && rm -rf build
mkdir build
cd build

CROSS_COMPILE="${TOP}/host/bin/mipsel-linux-"

cmake -DCMAKE_C_COMPILER="${CROSS_COMPILE}gcc" \
        -DCMAKE_CXX_COMPILER="${CROSS_COMPILE}g++" \
        -DLIVE555_INCLUDE_DIR=${TOP}/staging/usr/include \
        -DLIVE555_LIB_DIR=${TOP}/target/usr/lib \
        ..

make
cd ..
