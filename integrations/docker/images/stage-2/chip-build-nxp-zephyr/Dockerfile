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
    && wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_linux-x86_64_minimal.tar.xz \
    && tar xvf zephyr-sdk-0.16.8_linux-x86_64_minimal.tar.xz \
    && rm -rf zephyr-sdk-0.16.8_linux-x86_64_minimal.tar.xz \
    && zephyr-sdk-0.16.8/setup.sh -t arm-zephyr-eabi \
    && pip3 install --break-system-packages -U --no-cache-dir west \
    && west init zephyrproject -m https://github.com/nxp-zephyr/nxp-zsdk.git --mr nxp-v3.7.0 \
    && cd zephyrproject \
    && west update -o=--depth=1 -n \
    && west zephyr-export \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/nxp-zephyr/zephyr-sdk-0.16.8/ /opt/nxp-zephyr/zephyr-sdk-0.16.8/
COPY --from=build /opt/nxp-zephyr/zephyrproject/ /opt/nxp-zephyr/zephyrproject/

WORKDIR /opt/nxp-zephyr

ENV ZEPHYR_NXP_BASE=/opt/nxp-zephyr/zephyrproject/zephyr
ENV ZEPHYR_NXP_SDK_INSTALL_DIR=/opt/nxp-zephyr/zephyr-sdk-0.16.8
