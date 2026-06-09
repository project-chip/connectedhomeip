ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION} AS build
LABEL org.opencontainers.image.source=https://github.com/project-chip/connectedhomeip

WORKDIR /opt/silabs/

RUN set -x \
    && ZEPHYR_SDK_VERSION=1.0.1 \
    # Zephyr Silabs commit hash silabs/dev branch June 3 2026:
    # v4.4.0 + 917 fixes and ieee802.15.4 drivers
    && ZEPHYR_SILABS_HASH=7a2673077f4d2fbec085d6bc43259e084a32d2b2 \
    && python3 -m venv venv \
    && . venv/bin/activate \
    && pip3 install west \
    # Initialize west with the specific commit of the zephyr-silabs repository
    # This commit corresponds to the version of the Zephyr SDK used in the project
    && west init -m https://github.com/SiliconLabsSoftware/zephyr-silabs  \
    && cd zephyr-silabs \
    && git checkout ${ZEPHYR_SILABS_HASH} \
    && cd ../ \
    && west update \
    && pip3 install -r zephyr/scripts/requirements.txt \
    && west blobs fetch \
    && mkdir -p /opt/silabs/sdk \
    && cd /opt/silabs/sdk \
    && wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz \
    && tar xvf zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz \
    && rm zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz \
    && cd zephyr-sdk-${ZEPHYR_SDK_VERSION} \
    && ./setup.sh -t arm-zephyr-eabi \
    && export ZEPHYR_SDK_INSTALL_DIR=/opt/silabs/sdk \
    && west zephyr-export \
    && : # last line

FROM ghcr.io/project-chip/chip-build:${VERSION}

COPY --from=build /opt/silabs/ /opt/silabs/

ENV SILABS_ZEPHYR_SDK_PATH=/opt/silabs
ENV ZEPHYR_BASE=/opt/silabs/zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/opt/silabs/sdk
