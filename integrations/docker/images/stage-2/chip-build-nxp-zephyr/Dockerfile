ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} AS build
LABEL org.opencontainers.image.source=https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    xz-utils \
    && : # last line

WORKDIR /opt/nxp-zephyr
RUN set -x \
    && ZEPHYR_SDK_VERSION=0.17.4 \
    && NXP_ZSDK_VERSION=nxp-v4.3.0 \
    && wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64.tar.xz | tar -xJ \
    && zephyr-sdk-${ZEPHYR_SDK_VERSION}/setup.sh -t arm-zephyr-eabi \
    && pip3 install --break-system-packages -U --no-cache-dir west \
    && west init zephyrproject -m https://github.com/nxp-zephyr/nxp-zsdk.git --mr ${NXP_ZSDK_VERSION} \
    && cd zephyrproject \
    && west update -o=--depth=1 -n \
    && west zephyr-export \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/nxp-zephyr/zephyr-sdk-0.17.4/ /opt/nxp-zephyr/zephyr-sdk-0.17.4/
COPY --from=build /opt/nxp-zephyr/zephyrproject/ /opt/nxp-zephyr/zephyrproject/

WORKDIR /opt/nxp-zephyr

ENV ZEPHYR_NXP_BASE=/opt/nxp-zephyr/zephyrproject/zephyr
ENV ZEPHYR_NXP_SDK_INSTALL_DIR=/opt/nxp-zephyr/zephyr-sdk-0.17.4
