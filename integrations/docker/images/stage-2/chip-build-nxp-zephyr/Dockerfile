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
    && pip3 install --break-system-packages -U --no-cache-dir west \
    && west sdk install -t arm-zephyr-eabi --install-dir /opt/nxp-zephyr --version 0.17.0 \
    && west init zephyrproject -m https://github.com/nxp-zephyr/nxp-zsdk.git --mr nxp-v4.0.0 \
    && cd zephyrproject \
    && west update -o=--depth=1 -n \
    && west zephyr-export \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/nxp-zephyr/zephyr-sdk-0.17.0/ /opt/nxp-zephyr/zephyr-sdk-0.17.0/
COPY --from=build /opt/nxp-zephyr/zephyrproject/ /opt/nxp-zephyr/zephyrproject/

WORKDIR /opt/nxp-zephyr

ENV ZEPHYR_NXP_BASE=/opt/nxp-zephyr/zephyrproject/zephyr
ENV ZEPHYR_NXP_SDK_INSTALL_DIR=/opt/nxp-zephyr/zephyr-sdk-0.17.0
