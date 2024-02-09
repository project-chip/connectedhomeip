ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} as build
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# Setup toolchain
WORKDIR /opt/telink
RUN set -x \
    && curl --location https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.1/zephyr-sdk-0.16.1_linux-x86_64_minimal.tar.xz --output zephyr-sdk-0.16.1_linux-x86_64_minimal.tar.xz \
    && tar xvf zephyr-sdk-0.16.1_linux-x86_64_minimal.tar.xz \
    && rm -rf zephyr-sdk-0.16.1_linux-x86_64_minimal.tar.xz \
    && zephyr-sdk-0.16.1/setup.sh -t riscv64-zephyr-elf \
    && : # last line

# Setup Zephyr
ARG ZEPHYR_REVISION=c4dd151a1ffa96bfbd577ddc88a55c5785279514
WORKDIR /opt/telink/zephyrproject
RUN set -x \
    && python3 -m pip install -U --no-cache-dir west \
    && git clone https://github.com/telink-semi/zephyr \
    && cd zephyr \
    && git reset ${ZEPHYR_REVISION} --hard \
    && west init -l \
    && cd .. \
    && west update -o=--depth=1 -n -f smart \
    && west blobs fetch hal_telink \
    && west zephyr-export \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/telink/zephyr-sdk-0.16.1/ /opt/telink/zephyr-sdk-0.16.1/
COPY --from=build /opt/telink/zephyrproject/ /opt/telink/zephyrproject/

RUN set -x \
    && apt-get update \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && pip3 install --no-cache-dir --user -r /opt/telink/zephyrproject/zephyr/scripts/requirements.txt \
    && : # last line

ENV TELINK_ZEPHYR_BASE=/opt/telink/zephyrproject/zephyr
ENV TELINK_ZEPHYR_SDK_DIR=/opt/telink/zephyr-sdk-0.16.1
