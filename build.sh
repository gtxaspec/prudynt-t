#!/bin/bash
set -e

PRUDYNT_CROSS="${PRUDYNT_CROSS:-mipsel-linux-}"
TOP=$(pwd)

prudynt(){
	echo "Build prudynt"

	make clean
	/usr/bin/make -j13 \
	ARCH= CROSS_COMPILE="${PRUDYNT_CROSS}" \
	CFLAGS="-DPLATFORM_$1 -Os -DALLOW_RTSP_SERVER_PORT_REUSE=1 -DNO_OPENSSL=1 -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD \
	-I./3rdparty/install/include \
	-I./3rdparty/install/include/freetype2 \
	-I./3rdparty/install/include/liveMedia \
	-I./3rdparty/install/include/groupsock \
	-I./3rdparty/install/include/UsageEnvironment \
	-I./3rdparty/install/include/BasicUsageEnvironment" \
	LDFLAGS=" -L./3rdparty/install/lib $2" \
	-C $PWD all
	exit 0
}

deps() {
	rm -rf 3rdparty
	mkdir -p 3rdparty/install

	echo "Build freetype2"
	cd 3rdparty
	rm -rf freetype
	if [[ ! -f freetype-2.13.2.tar.xz ]]; then
		wget 'https://download-mirror.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.xz'
	fi
	tar xf freetype-2.13.2.tar.xz
	mv freetype-2.13.2 freetype
	cd freetype
	CC="${PRUDYNT_CROSS}gcc" ./configure --host mipsel-linux-gnu --prefix="$TOP/3rdparty/install/" --without-harfbuzz --disable-largefile --disable-mmap --without-png --without-brotli --without-zlib --without-bzip2
	make -j$(nproc)
	make install
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
	rm -rf live555
	if [[ ! -f live555-latest.tar.gz ]]; then
		wget 'http://www.live555.com/liveMedia/public/live555-latest.tar.gz';
	fi
	tar xf live555-latest.tar.gz
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
		cp ../../res/live555-config.prudynt ./config.prudynt
		./genMakefiles prudynt
	fi

	PRUDYNT_ROOT="${TOP}" PRUDYNT_CROSS="${PRUDYNT_CROSS}" make -j$(nproc)
	PRUDYNT_ROOT="${TOP}" PRUDYNT_CROSS="${PRUDYNT_CROSS}" make install
	cd ../../

	echo "import libimp"
	cd 3rdparty
	rm -rf ingenic-lib
	if [[ ! -d ingenic-lib ]]; then
	git clone https://github.com/gtxaspec/ingenic-lib

	case "$1" in
		T10)
			echo "use T10 libs"
			cp ingenic-lib/T10/lib/3.12.0/uclibc/4.7.2/* $TOP/3rdparty/install/lib
			;;
		T20)
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
			#cp ingenic-lib/$1/lib/1.1.6/uclibc/5.4.0/* $TOP/3rdparty/install/lib
			cp ingenic-lib/$1/lib/1.1.5/uclibc/4.7.2/* $TOP/3rdparty/install/lib
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
	git clone https://github.com/gtxaspec/ingenic-musl
	cd ingenic-musl
	if [[ "$2" == "-static" ]]; then
		make CC="${PRUDYNT_CROSS}gcc" static
	else
		make CC="${PRUDYNT_CROSS}gcc"
	fi
	cp libmuslshim.* ../install/lib/
	fi
	cd ..
}

if [ $# -eq 0 ]; then
	echo "Usage: ./build.sh deps <platform> <-static>"
	echo "Usage: ./build.sh prudynt <platform> <-static>"
	echo "Platform: T20/T21/T23/T30/T31"
	exit 1
elif [[ "$1" == "deps" ]]; then
	deps $2 $3
elif [[ "$1" == "prudynt" ]]; then
	prudynt $2 $3
fi
