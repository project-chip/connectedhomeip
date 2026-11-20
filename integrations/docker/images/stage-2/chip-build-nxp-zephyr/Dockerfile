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
    && ZEPHYR_SDK_VERSION=1.0.1 \
    && NXP_ZEPHYR_VERSION=f8da8ea370508f274685a25ff19dbe5a9ff54790 \
    && wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz | tar -xJ \
    && zephyr-sdk-${ZEPHYR_SDK_VERSION}/setup.sh -t arm-zephyr-eabi \
    && pip3 install --break-system-packages -U --no-cache-dir west \
    && mkdir -p zephyrproject/zephyr \
    && cd zephyrproject/zephyr \
    && git init \
    && git remote add origin https://github.com/zephyrproject-rtos/zephyr.git \
    && git fetch --depth=1 origin ${NXP_ZEPHYR_VERSION} \
    && git reset --hard ${NXP_ZEPHYR_VERSION} \
    && cd .. \
    && west init -l zephyr \
    && west update -o=--depth=1 -n \
    && pip3 install --break-system-packages --no-cache-dir -r zephyr/scripts/requirements-base.txt \
    && west zephyr-export \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/nxp-zephyr/zephyr-sdk-1.0.1/ /opt/nxp-zephyr/zephyr-sdk-1.0.1/
COPY --from=build /opt/nxp-zephyr/zephyrproject/ /opt/nxp-zephyr/zephyrproject/

WORKDIR /opt/nxp-zephyr

ENV ZEPHYR_NXP_BASE=/opt/nxp-zephyr/zephyrproject/zephyr
ENV ZEPHYR_NXP_SDK_INSTALL_DIR=/opt/nxp-zephyr/zephyr-sdk-1.0.1
