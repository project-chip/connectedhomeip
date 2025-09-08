ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} AS build
LABEL org.opencontainers.image.source=https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    xz-utils \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && : # last line

WORKDIR /opt/realtek-zephyr
RUN set -x \
    && wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_linux-x86_64_minimal.tar.xz \
    && tar xvf zephyr-sdk-0.16.8_linux-x86_64_minimal.tar.xz \
    && rm -rf zephyr-sdk-0.16.8_linux-x86_64_minimal.tar.xz \
    && zephyr-sdk-0.16.8/setup.sh -t arm-zephyr-eabi \
    && pip3 install --break-system-packages -U --no-cache-dir west \
    && west init -m https://github.com/rtkconnectivity/realtek-zephyr-project zephyr-rtk-project \
    && cd zephyr-rtk-project \
    && west update -o=--depth=1 -n \
    && west zephyr-export \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/realtek-zephyr/zephyr-sdk-0.16.8/ /opt/realtek-zephyr/zephyr-sdk-0.16.8/
COPY --from=build /opt/realtek-zephyr/zephyr-rtk-project/ /opt/realtek-zephyr/zephyr-rtk-project/

WORKDIR /opt/realtek-zephyr

ENV ZEPHYR_REALTEK_BASE=/opt/realtek-zephyr/zephyr-rtk-project/zephyr
ENV ZEPHYR_REALTEK_SDK_INSTALL_DIR=/opt/realtek-zephyr/zephyr-sdk-0.16.8
