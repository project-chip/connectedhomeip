ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    gcc-arm-none-eabi=15:9-2019-q4-0ubuntu1 \
    binutils-arm-none-eabi=2.34-4ubuntu1+13ubuntu1 \
    ccache=3.7.7-1 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line
