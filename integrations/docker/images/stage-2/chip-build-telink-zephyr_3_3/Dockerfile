ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    xz-utils \
    && : # last line

# Setup toolchain
WORKDIR /opt/telink
RUN set -x \
    && curl --location https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.17.0/zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz --output zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz \
    && tar xvf zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz \
    && rm -rf zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz \
    && zephyr-sdk-0.17.0/setup.sh -t riscv64-zephyr-elf \
    && : # last line

# Setup Zephyr version: 3.3.0; branch: develop_3.3
ARG ZEPHYR_REVISION=fdccaba1ebc147908e9cca653e6c6743def3332b
WORKDIR /opt/telink/zephyrproject
RUN set -x \
    && python3 -m pip install --break-system-packages -U --no-cache-dir west \
    && git init zephyr \
    && cd zephyr \
    && git remote add origin https://github.com/telink-semi/zephyr \
    && git fetch --depth=1 origin ${ZEPHYR_REVISION} \
    && git reset ${ZEPHYR_REVISION} --hard \
    && west init -l \
    && west update -o=--depth=1 -n -f smart \
    && west blobs fetch hal_telink \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/telink/zephyr-sdk-0.17.0/ /opt/telink/zephyr-sdk-0.17.0/
COPY --from=build /opt/telink/zephyrproject/ /opt/telink/zephyrproject/

RUN set -x \
    && apt-get update \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && pip3 install --break-system-packages --no-cache-dir --user -r /opt/telink/zephyrproject/zephyr/scripts/requirements.txt \
    && : # last line

ENV TELINK_ZEPHYR_BASE=/opt/telink/zephyrproject/zephyr
ENV TELINK_ZEPHYR_SDK_DIR=/opt/telink/zephyr-sdk-0.17.0
