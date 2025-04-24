FROM --platform=linux/amd64 debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /src

RUN apt-get update && \
    apt-get install -y --no-install-recommends --no-install-suggests \
    build-essential bc bison cpio curl file flex git libncurses-dev \
    make rsync unzip wget whiptail gcc lzop u-boot-tools \
    ca-certificates cmake && \
    rm -rf /var/lib/apt/lists/*

ENV TOOLCHAIN_URL=https://github.com/themactep/thingino-firmware/releases/download/toolchain-x86_64/thingino-toolchain-x86_64_xburst1_4_4_musl_gcc14-linux-mipsel.tar.gz
ENV TOOLCHAIN_DIR=/opt/mipsel-thingino-linux-musl_sdk-buildroot

RUN mkdir -p /opt && \
    cd /opt && \
    wget "$TOOLCHAIN_URL" -O thingino-toolchain.tar.gz && \
    tar -xf thingino-toolchain.tar.gz && \
    cd mipsel-thingino-linux-musl_sdk-buildroot && \
    ./relocate-sdk.sh

ENV PATH="${TOOLCHAIN_DIR}/bin:${PATH}"
ENV CC="${TOOLCHAIN_DIR}/bin/mipsel-linux-gcc"
ENV CXX="${TOOLCHAIN_DIR}/bin/mipsel-linux-g++"

ARG TARGET=T31
ARG BUILD_TYPE=static

ENV TARGET=${TARGET}
ENV BUILD_TYPE=${BUILD_TYPE}

COPY . /deps

RUN  bash -c 'case "$BUILD_TYPE" in \
      static) \
        SUFFIX="-static";; \
      hybrid) \
        SUFFIX="-hybrid";; \
      *) \
        SUFFIX="";; \
    esac && export SUFFIX'

RUN cd /deps && PRUDYNT_CROSS="mipsel-linux-" ./build.sh deps $TARGET $SUFFIX

CMD cp -r /deps/3rdparty /src && \
    PRUDYNT_CROSS="mipsel-linux-" ./build.sh prudynt $TARGET $SUFFIX && \
    mv bin/prudynt "bin/prudynt-${TARGET}-${BUILD_TYPE}"
