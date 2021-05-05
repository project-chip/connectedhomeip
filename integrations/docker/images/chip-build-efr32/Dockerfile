ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}




# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    ccache
