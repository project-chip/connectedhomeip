ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}




# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    ccache

#Clone Gecko SDK
RUN git clone https://github.com/SiliconLabs/sdk_support.git

ENV EFR32_SDK_ROOT=/sdk_support/