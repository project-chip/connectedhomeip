ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source=https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    xz-utils \
    && : # last line

ARG NXP_ZEPHYR_REVISION=70be2ff0b565a3313128f5577f51cfeb3ebcf602

WORKDIR /opt/nxp-zephyr
RUN set -x \
    && ZEPHYR_SDK_VERSION=1.0.1 \
    && wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz | tar -xJ \
    && zephyr-sdk-${ZEPHYR_SDK_VERSION}/setup.sh -t arm-zephyr-eabi \
    && pip3 install --break-system-packages -U --no-cache-dir west \
    && mkdir -p zephyrproject/zephyr \
    && cd zephyrproject/zephyr \
    && git init \
    && git remote add origin https://github.com/zephyrproject-rtos/zephyr.git \
    && git fetch --depth=1 origin ${NXP_ZEPHYR_REVISION} \
    && git reset ${NXP_ZEPHYR_REVISION} --hard \
    && cd /opt/nxp-zephyr/zephyrproject \
    && west init -l zephyr \
    && west update -o=--depth=1 -n \
    && west zephyr-export \
    && : # last line

ENV ZEPHYR_NXP_BASE=/opt/nxp-zephyr/zephyrproject/zephyr
ENV ZEPHYR_NXP_SDK_INSTALL_DIR=/opt/nxp-zephyr/zephyr-sdk-1.0.1
