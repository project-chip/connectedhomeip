ARG chip_build_VERSION
FROM connectedhomeip/chip-build:${chip_build_VERSION}

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    ccache

#Clone Gecko SDK
RUN mkdir -p /opt/SiliconLabs && git clone https://github.com/SiliconLabs/sdk_support.git /opt/SiliconLabs/sdk_support

ENV EFR32_SDK_ROOT=/opt/SiliconLabs/sdk_support
