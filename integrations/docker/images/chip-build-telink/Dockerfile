ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION} as build

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    wget=1.20.3-1ubuntu2 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Setup toolchain
RUN set -x \
    && wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.13.2/zephyr-toolchain-riscv64-0.13.2-linux-x86_64-setup.run -O /tmp/zephyr-toolchain-riscv64-setup.run \
    && chmod +x /tmp/zephyr-toolchain-riscv64-setup.run \
    && /tmp/zephyr-toolchain-riscv64-setup.run -- -d /opt/telink/zephyr-sdk-0.13.2 \
    && : # last line

# Setup Zephyr
ARG ZEPHYR_REVISION=b522ea774ab31c3e636f25da374b43855e9d4e60
WORKDIR /opt/telink/zephyrproject
RUN set -x \
    && python3 -m pip install -U --no-cache-dir \
    west==0.11.1 \
    && git clone https://github.com/rikorsev/zephyr \
    && cd zephyr \
    && git reset ${ZEPHYR_REVISION} --hard \
    && west init -l \
    && cd .. \
    && west update \
    && cd modules/hal/telink \
    && git remote add telink https://github.com/rikorsev/hal_telink \
    && git fetch telink telink_crypto \
    && git checkout telink_crypto \
    && west zephyr-export \
    && : # last line

FROM connectedhomeip/chip-build:${VERSION}

COPY --from=build /opt/telink/zephyr-sdk-0.13.2/ /opt/telink/zephyr-sdk-0.13.2/
COPY --from=build /opt/telink/zephyrproject/ /opt/telink/zephyrproject/

ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/opt/telink/zephyr-sdk-0.13.2

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    ccache=3.7.7-1 \
    dfu-util=0.9-1 \
    device-tree-compiler=1.5.1-1 \
    gcc-multilib=4:9.3.0-1ubuntu2 \
    g++-multilib=4:9.3.0-1ubuntu2 \
    libsdl2-dev=2.0.10+dfsg1-3 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/ \
    && python3 -m pip install -U --no-cache-dir \
    pyelftools==0.27 \
    && pip3 install --no-cache-dir --user -r /opt/telink/zephyrproject/zephyr/scripts/requirements.txt \
    && : # last line

ARG ZEPHYR_PROJECT_DIR=/opt/telink/zephyrproject
ENV TELINK_ZEPHYR_BASE=${ZEPHYR_PROJECT_DIR}/zephyr
ENV ZEPHYR_BASE=${ZEPHYR_PROJECT_DIR}/zephyr
