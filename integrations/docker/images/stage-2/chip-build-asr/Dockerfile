ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Setup toolchain for ASR582X
ARG ASR_DIR=/opt/asr
RUN set -x \
    && mkdir ${ASR_DIR} \
    && cd ${ASR_DIR} \
    && mkdir -p Tools/toolchain \
    && cd Tools/toolchain \
    && wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/RC2.1/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 \
    && tar -jxvf gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 \
    && rm gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 \
    && : # last line

ENV ASR_TOOLCHAIN_PATH=${ASR_DIR}/Tools/toolchain/gcc-arm-none-eabi-9-2019-q4-major/bin/
