#!/bin/bash
set -e
: "${PRUDYNT_CROSS:=ccache mipsel-linux-}"
TOP=$(pwd)

prudynt(){
	echo "Build prudynt"

	cd $TOP
	make clean

	if [ "$2" = "-static" ]; then
		BIN_TYPE="-DBINARY_STATIC"
	elif [ "$2" = -"hybrid" ]; then
		BIN_TYPE="-DBINARY_HYBRID"
	else
		BIN_TYPE="-DBINARY_DYNAMIC"
	fi

	/usr/bin/make -j$(nproc) \
	ARCH= CROSS_COMPILE="${PRUDYNT_CROSS}" \
	CFLAGS="-DPLATFORM_$1 $BIN_TYPE -O2 -DALLOW_RTSP_SERVER_PORT_REUSE=1 -DNO_OPENSSL=1 \
	-isystem ./3rdparty/install/include \
	-isystem ./3rdparty/install/include/liveMedia \
	-isystem ./3rdparty/install/include/groupsock \
	-isystem ./3rdparty/install/include/UsageEnvironment \
	-isystem ./3rdparty/install/include/BasicUsageEnvironment" \
	LDFLAGS=" -L./3rdparty/install/lib" \
	-C $PWD all
	exit 0
}

deps() {
	rm -rf 3rdparty
	mkdir -p 3rdparty/install
	mkdir -p 3rdparty/install/include
	CROSS_COMPILE=${PRUDYNT_CROSS}

	echo "Build libhelix-aac"
	cd 3rdparty
	if [[ "$2" == "-static" ]]; then
		PRUDYNT_CROSS=$PRUDYNT_CROSS ../scripts/make_libhelixaac_deps.sh -static
	else
		PRUDYNT_CROSS=$PRUDYNT_CROSS ../scripts/make_libhelixaac_deps.sh
	fi
	cd ../

	echo "Build libwebsockets"
	cd 3rdparty
	if [[ "$2" == "-static" ]]; then
		PRUDYNT_CROSS=$PRUDYNT_CROSS ../scripts/make_libwebsockets_deps.sh -static
	else
		PRUDYNT_CROSS=$PRUDYNT_CROSS ../scripts/make_libwebsockets_deps.sh
	fi
	cd ../

	echo "Build opus"
	cd 3rdparty
	if [[ "$2" == "-static" ]]; then
		PRUDYNT_CROSS=$PRUDYNT_CROSS ../scripts/make_opus_deps.sh -static
	else
		PRUDYNT_CROSS=$PRUDYNT_CROSS ../scripts/make_opus_deps.sh
	fi
	cd ../

	echo "Build libschrift"
	cd 3rdparty
	rm -rf libschrift
	git clone --depth=1 https://github.com/tomolt/libschrift/
	cd libschrift
	git apply ../../res/libschrift.patch
	mkdir -p $TOP/3rdparty/install/lib
	mkdir -p $TOP/3rdparty/install/include
	if [[ "$2" == "-static" ]]; then
		${PRUDYNT_CROSS}gcc -std=c99 -pedantic -Wall -Wextra -Wconversion -c -o schrift.o schrift.c
		${PRUDYNT_CROSS}ar rc libschrift.a schrift.o
		${PRUDYNT_CROSS}ranlib libschrift.a
		cp libschrift.a $TOP/3rdparty/install/lib/
	else
		${PRUDYNT_CROSS}gcc -std=c99 -pedantic -Wall -Wextra -Wconversion -fPIC -c -o schrift.o schrift.c
		${PRUDYNT_CROSS}gcc -shared -o libschrift.so schrift.o
		cp libschrift.so $TOP/3rdparty/install/lib/
	fi
	cp schrift.h $TOP/3rdparty/install/include/
	cd ../../

	echo "Build libconfig"
	cd 3rdparty
	rm -rf libconfig
	if [[ ! -f libconfig-1.7.3.tar.gz ]]; then
		wget 'https://hyperrealm.github.io/libconfig/dist/libconfig-1.7.3.tar.gz';
	fi
	tar xf libconfig-1.7.3.tar.gz
	mv libconfig-1.7.3 libconfig
	cd libconfig
	CC="${PRUDYNT_CROSS}gcc" CXX="${PRUDYNT_CROSS}g++" ./configure --host mipsel-linux-gnu --prefix="$TOP/3rdparty/install"
	make -j$(nproc)
	make install
	cd ../../

	echo "Build live555"
	cd 3rdparty
	rm -rf live
	git clone https://github.com/themactep/live555.git live
	cd live

	if [[ -f Makefile ]]; then
		make distclean
	fi

	if [[ "$2" == "-static" ]]; then
		echo "STATIC LIVE555"
		cp ../../res/live555-config.prudynt-static ./config.prudynt-static
		./genMakefiles prudynt-static
	else
		echo "SHARED LIVE555"
		patch config.linux-with-shared-libraries ../../res/live555-prudynt.patch --output=./config.prudynt
		./genMakefiles prudynt
	fi

	PRUDYNT_ROOT="${TOP}" PRUDYNT_CROSS="${PRUDYNT_CROSS}" make -j$(nproc)
	PRUDYNT_ROOT="${TOP}" PRUDYNT_CROSS="${PRUDYNT_CROSS}" make install
	cd ../../

	echo "import libimp"
	cd 3rdparty
	rm -rf ingenic-lib
	if [[ ! -d ingenic-lib ]]; then
	git clone --depth=1 https://github.com/gtxaspec/ingenic-lib

	case "$1" in
		T10|T20)
			echo "use T20 libs"
			cp ingenic-lib/T20/lib/3.12.0/uclibc/4.7.2/* $TOP/3rdparty/install/lib
			;;
		T21)
			echo "use $1 libs"
			cp ingenic-lib/$1/lib/1.0.33/uclibc/5.4.0/* $TOP/3rdparty/install/lib
			;;
		T23)
			echo "use $1 libs"
			cp ingenic-lib/$1/lib/1.1.0/uclibc/5.4.0/* $TOP/3rdparty/install/lib
			;;
		T30)
			echo "use $1 libs"
			cp ingenic-lib/$1/lib/1.0.5/uclibc/5.4.0/* $TOP/3rdparty/install/lib
			;;
		T31)
			echo "use $1 libs"
			cp ingenic-lib/$1/lib/1.1.6/uclibc/5.4.0/* $TOP/3rdparty/install/lib
			;;
		C100)
			echo "use $1 libs"
			cp ingenic-lib/$1/lib/2.1.0/uclibc/5.4.0/* $TOP/3rdparty/install/lib
			;;
		T40)
			echo "use $1 libs"
			cp ingenic-lib/$1/lib/1.2.0/uclibc/* $TOP/3rdparty/install/lib
			;;
		T41)
			echo "use $1 libs"
			cp ingenic-lib/$1/lib/1.2.0/uclibc/* $TOP/3rdparty/install/lib
			;;
		*)
			echo "Unsupported or unspecified SoC model."
			;;
	esac
	fi

	cd ../

	echo "import libmuslshim"
	cd 3rdparty
	rm -rf ingenic-musl
	if [[ ! -d ingenic-musl ]]; then
	git clone --depth=1 https://github.com/gtxaspec/ingenic-musl
	cd ingenic-musl
	if [[ "$2" == "-static" ]]; then
		make CC="${PRUDYNT_CROSS}gcc" -j$(nproc) static
		make CC="${PRUDYNT_CROSS}gcc" -j$(nproc)
	else
		make CC="${PRUDYNT_CROSS}gcc" -j$(nproc)
	fi
	cp libmuslshim.* ../install/lib/
	fi
	cd $TOP

	echo "import libaudioshim"
	cd 3rdparty
	rm -rf libaudioshim
	if [[ ! -d libaudioshim ]]; then
	git clone --depth=1 https://github.com/gtxaspec/libaudioshim
	cd libaudioshim
		make CC="${PRUDYNT_CROSS}gcc" -j$(nproc)
	cp libaudioshim.* ../install/lib/
	fi
	cd $TOP

	echo "Build faac"
	cd 3rdparty
	rm -rf faac
	git clone --depth=1 https://github.com/knik0/faac.git
	cd faac
	sed -i 's/^#define MAX_CHANNELS 64/#define MAX_CHANNELS 2/' libfaac/coder.h
	./bootstrap
	if [[ "$2" == "-static" ]]; then
		CC="${PRUDYNT_CROSS}gcc" ./configure --host mipsel-linux-gnu --prefix="$TOP/3rdparty/install" --enable-static --disable-shared
	else
		CC="${PRUDYNT_CROSS}gcc" ./configure --host mipsel-linux-gnu --prefix="$TOP/3rdparty/install" --disable-static --enable-shared
	fi
	make -j$(nproc)
	make install
	cd ../../
}

if [ $# -eq 0 ]; then
	echo "Standalone Prudynt Build"
	echo "Usage: ./build.sh deps <platform> [options]"
	echo "       ./build.sh prudynt <platform> [options]"
	echo "       ./build.sh full <platform> [options]"
	echo ""
	echo "Platforms: T20, T21, T23, T30, T31, C100, T40, T41"
	echo "Options:   -static (optional, for static builds)"
	exit 1
elif [[ "$1" == "deps" ]]; then
	deps $2 $3
elif [[ "$1" == "prudynt" ]]; then
	prudynt $2 $3
elif [[ "$1" == "full" ]]; then
	deps $2 $3
	prudynt $2 $3
fi
